/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/WebUIService/Handlers/DebugStatisticsHandler.h"
#include "Server/Server.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DebugObjects.h"

#include <ctime>

DebugStatisticsHandler::DebugStatisticsHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void DebugStatisticsHandler::Register(CivetServer* Server)
{
    Server->addHandler("/debug_statistics", this);
}

void DebugStatisticsHandler::GatherData()
{
    std::scoped_lock lock(DataMutex);

}

bool DebugStatisticsHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    {
        std::scoped_lock lock(DataMutex);

        auto timers = nlohmann::json::array();
        for (DebugTimer* Timer : DebugTimer::GetTimers())
        {
            auto stat = nlohmann::json::object();
            stat["name"] = Timer->GetName();
            stat["current"] = StringFormat("%.2f ms", Timer->GetCurrent() * 1000.0);
            stat["average"] = StringFormat("%.2f ms", Timer->GetAverage() * 1000.0);
            stat["peak"] = StringFormat("%.2f ms", Timer->GetPeak() * 1000.0);
            timers.push_back(stat);
        }
        
        auto counters = nlohmann::json::array();
        for (DebugCounter* Counter : DebugCounter::GetCounters())
        {
            auto stat = nlohmann::json::object();
            stat["name"] = Counter->GetName();
            stat["average_rate"] = StringFormat("%.2f", Counter->GetAverageRate());
            stat["total_lifetime"] = StringFormat("%.2f", Counter->GetTotalLifetime());
            counters.push_back(stat);
        }

        auto logs = nlohmann::json::array();
        if (Service->GetServer()->IsDefaultServer())
        {
            for (const LogMessage& Message : GetRecentLogs())
            {
                auto stat = nlohmann::json::object();
                stat["level"] = Message.Level;
                stat["source"] = Message.Source;
                stat["message"] = Message.Message;
                logs.push_back(stat);
            }
        }

        json["timers"] = timers;
        json["counters"] = counters;
        json["logs"] = logs;
    }

    RespondJson(Connection, json);

    MarkAsNeedsDataGather();

    return true;
}
