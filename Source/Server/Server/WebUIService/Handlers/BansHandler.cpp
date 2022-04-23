/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/WebUIService/Handlers/BansHandler.h"
#include "Server/Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

BansHandler::BansHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void BansHandler::Register(CivetServer* Server)
{
    Server->addHandler("/bans", this);
}

bool BansHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    ServerDatabase& Database = Service->GetServer()->GetDatabase();
    std::vector<std::string> BannedSteamIds = Database.GetBannedSteamIds();

    nlohmann::json json;
    {
        auto bansArray = nlohmann::json::array();
        for (std::string& SteamId : BannedSteamIds)
        {
            auto banJson = nlohmann::json::object();
            
            uint64_t SteamId64;
            sscanf(SteamId.c_str(), "%016llx", &SteamId64);

            banJson["steamId64"] = std::to_string(SteamId64);            
            banJson["steamId"] = SteamId;
            std::string reason = "Manual";

            std::vector<AntiCheatLog> logs = Database.GetAntiCheatLogs(SteamId);
            if (!logs.empty())
            {
                reason = "";

                for (AntiCheatLog& log : logs)
                {
                    if (!reason.empty())
                    {
                        reason += "\n";
                    }
                    reason += StringFormat("%s: %s", log.TriggerName.c_str(), log.Extra.c_str());                    
                }
            }

            banJson["reason"] = reason;
            bansArray.push_back(banJson);
        }

        json["bans"] = bansArray;   
    }

    RespondJson(Connection, json);

    return true;
}

bool BansHandler::handleDelete(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    if (!ReadJson(Server, Connection, json) ||
        !json.contains("steamId"))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    std::string SteamId = json["steamId"];

    ServerDatabase& Database = Service->GetServer()->GetDatabase();

    LogS("WebUI", "Unbanning player: %s", SteamId.c_str());
    Database.UnbanPlayer(SteamId);

    nlohmann::json responseJson;
    RespondJson(Connection, responseJson);

    return true;
}