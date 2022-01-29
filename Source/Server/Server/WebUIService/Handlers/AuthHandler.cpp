/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Server/WebUIService/Handlers/AuthHandler.h"

#include "Core/Utils/Logging.h"

AuthHandler::AuthHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
}


void AuthHandler::Register(CivetServer* Server)
{
    Server->addHandler("/auth", this);
}

bool AuthHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    RespondJson(Connection, json);

    return true;
}

bool AuthHandler::handlePost(CivetServer* Server, struct mg_connection* Connection)
{
    nlohmann::json json;
    if (!ReadJson(Server, Connection, json) ||
        !json.contains("username") ||
        !json.contains("password"))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    std::string Username = json["username"];
    std::string Password = json["password"];

    std::string CorrectUsername = Service->GetServer()->GetConfig().WebUIServerUsername;
    std::string CorrectPassword = Service->GetServer()->GetConfig().WebUIServerPassword;

    if (Username == CorrectUsername &&
        Password == CorrectPassword &&
        !CorrectUsername.empty() &&
        !CorrectPassword.empty())
    {
        nlohmann::json json;
        json["token"] = Service->AddAuthToken();

        LogS("WebUI", "User has logged in to webui.");

        RespondJson(Connection, json);
    }
    else
    {
        mg_send_http_error(Connection, 401, "Token login.");
        return true;
    }

    return true;
}
