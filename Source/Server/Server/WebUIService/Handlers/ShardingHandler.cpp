/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/WebUIService/Handlers/ShardingHandler.h"
#include "Server/Server.h"
#include "Server/ServerManager.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include "ThirdParty/civetweb/include/civetweb.h"
#include "ThirdParty/civetweb/include/CivetServer.h"

#include <thread>
#include <condition_variable>

ShardingHandler::ShardingHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void ShardingHandler::Register(CivetServer* Server)
{
    Server->addHandler("/sharding", this);
}

bool ShardingHandler::handlePost(CivetServer* WebServer, struct mg_connection* Connection)
{
    ServerManager& Manager = Service->GetServer()->GetManager();

    nlohmann::json json;
    if (!ReadJson(WebServer, Connection, json) ||
        !json.contains("serverName") ||
        !json.contains("serverPassword") ||
        !json.contains("serverGameType") ||
        !json.contains("machineId"))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    std::string ServerName = json["serverName"];
    std::string ServerPassword = json["serverPassword"];
    std::string ServerGameType = json["serverGameType"];
    std::string MachineId = json["machineId"];

    std::string UserIp = mg_get_request_info(Connection)->remote_addr;
    if (size_t Pos = UserIp.find(":"); Pos != std::string::npos)
    {
        UserIp = UserIp.substr(0, Pos);
    }
    std::string RequestHash = UserIp + "|" + MachineId;
    std::string ServerId = "";

    Server* Instance = nullptr;

    GameType ServerEnumGameType = GameType::Unknown;
    if (!ParseGameType(ServerGameType.c_str(), ServerEnumGameType))
    {
        mg_send_http_error(Connection, 400, "Malformed body, unknown game type.");
        return true;
    }

    // Find existing server created by user, or created a new one.
    if (auto Iter = RequestHashToServerId.find(RequestHash); Iter != RequestHashToServerId.end())
    {
        ServerId = Iter->second;
        Instance = Manager.FindServer(ServerId);
    }

    if (Instance == nullptr)
    {
        std::mutex mutex;
        std::condition_variable convar;
        bool Success = false;

        std::unique_lock lock(mutex);

        Manager.QueueCallback([&Manager, ServerName, ServerPassword, &ServerId, &ServerEnumGameType, &Success, &mutex, &convar]() mutable {
            std::unique_lock inner_lock(mutex);
            Success = Manager.NewServer(ServerName, ServerPassword, ServerEnumGameType, ServerId);
            convar.notify_all();
        });

        convar.wait(lock);

        if (!Success)
        {
            mg_send_http_error(Connection, 500, "Failed to start server.");
            return false;
        }

        Instance = Manager.FindServer(ServerId);
        if (!Instance)
        {
            mg_send_http_error(Connection, 500, "Failed to find server.");
            return false;
        }
    }

    RequestHashToServerId[RequestHash] = ServerId;

    // Send back a response with the login details for the server.
    const RuntimeConfig& Config = Instance->GetConfig();

    std::string Hostname = Config.ServerHostname.length() > 0 ? Config.ServerHostname : Instance->GetPublicIP().ToString();

    nlohmann::json responseJson;
    responseJson["id"] = Instance->GetId();
    responseJson["webUsername"] = Config.WebUIServerUsername;
    responseJson["webPassword"] = Config.WebUIServerPassword;
    responseJson["webUrl"] = StringFormat("http://%s:%i/", Hostname.c_str(), Config.WebUIServerPort);
    RespondJson(Connection, responseJson);

    return true;
}
