/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/WebUIService/Handlers/WebUIHandler.h"

#include "Platform/Platform.h"

#include "Config/BuildConfig.h"

WebUIHandler::WebUIHandler(WebUIService* InService)
    : Service(InService)
{
}

void WebUIHandler::RespondJson(struct mg_connection* Connection, nlohmann::json& Json)
{
    std::string result = Json.dump(4);

    mg_send_http_ok(Connection, "application/json; charset=utf-8", result.size());
    mg_write(Connection, result.data(), result.size());
}

bool WebUIHandler::ReadJson(CivetServer* Server, struct mg_connection* Connection, nlohmann::json& Json)
{
    std::string Body = Server->getPostData(Connection);

    try
    {
        Json = nlohmann::json::parse(Body);
    }
    catch (nlohmann::json::parse_error)
    {
        return false;
    }

    return true;
}

bool WebUIHandler::NeedsDataGather()
{
    double Time = GetSeconds();

    if (Time - LastDataGather < BuildConfig::WEBUI_GATHER_DATA_MIN_INTERVAL)
    {
        return false;
    }

    if (Time - LastMarkedAsNeedingDataGather > BuildConfig::WEBUI_GATHER_DATA_TIMEOUT)
    {
        return false;
    }

    LastDataGather = GetSeconds();

    return true;
}

void WebUIHandler::MarkAsNeedsDataGather()
{
    LastMarkedAsNeedingDataGather = GetSeconds();
}