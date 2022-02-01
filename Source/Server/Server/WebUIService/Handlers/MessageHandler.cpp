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
#include "Server/WebUIService/Handlers/MessageHandler.h"
#include "Server/Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

MessageHandler::MessageHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void MessageHandler::Register(CivetServer* Server)
{
    Server->addHandler("/message", this);
}

bool MessageHandler::handlePost(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    if (!ReadJson(Server, Connection, json) ||
        !json.contains("playerId") ||
        !json.contains("message"))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    uint32_t playerId = json["playerId"];
    std::string message = json["message"];

    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();
    if (playerId == 0)
    {
        LogS("WebUI", "Sending message to all players: %s", message.c_str());
        for (auto Client : Game->GetClients())
        {
            Client->SendTextMessage(message);
        }
    }
    else
    {
        if (std::shared_ptr<GameClient> Client = Game->FindClientByPlayerId(playerId))
        {
            LogS("WebUI", "Sending message to %s: %s", Client->GetName().c_str(), message.c_str());
            Client->SendTextMessage(message);
        }
    }

    nlohmann::json responseJson;
    RespondJson(Connection, responseJson);

    return true;
}
