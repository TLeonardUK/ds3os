/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/WebUIService/Handlers/StatisticsHandler.h"
#include "Server/Server.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/DarkSouls3/GameManagers/BloodMessage/BloodMessageManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Bloodstain/BloodstainManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/QuickMatch/QuickMatchManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Signs/SignManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/Ghosts/GhostManager.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include <ctime>

StatisticsHandler::StatisticsHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void StatisticsHandler::Register(CivetServer* Server)
{
    Server->addHandler("/statistics", this);
}

void StatisticsHandler::GatherData()
{
    std::scoped_lock lock(DataMutex);

    std::shared_ptr<GameService> Game = Service->GetServer()->GetService<GameService>();
    std::vector<std::shared_ptr<GameClient>> Clients = Game->GetClients();

    // Grab record of statistics over time.
    bool ShouldSample = Clients.size() != PreviousSampleClientSize;

    if (GetSeconds() >= MustSampleTime)
    {
        ShouldSample = true;
    }

    if (ShouldSample && GetSeconds() >= NextSampleTime)
    {
        time_t RawTime;
        struct tm* LocalTime;
        char Buffer[128];

        time(&RawTime);
        LocalTime = localtime(&RawTime);
        strftime(Buffer, sizeof(Buffer), "%R", LocalTime);

        Sample NewSample;
        NewSample.Timestamp = Buffer;
        NewSample.ActivePlayers = Clients.size();

        Samples.push_back(NewSample);
        NextSampleTime = GetSeconds() + MinSampleInterval;
        MustSampleTime = GetSeconds() + MaxSampleInterval;
        PreviousSampleClientSize = Clients.size();

        if (Samples.size() > MaxSamples)
        {
            Samples.erase(Samples.begin());
        }

        UniquePlayerCount = Service->GetServer()->GetDatabase().GetTotalPlayers();
    }

    // Grab some per-frame statistics.
    std::shared_ptr<BloodMessageManager> BloodMessages = Game->GetManager<BloodMessageManager>();
    std::shared_ptr<BloodstainManager> Bloodstains = Game->GetManager<BloodstainManager>();
    std::shared_ptr<QuickMatchManager> QuickMatches = Game->GetManager<QuickMatchManager>();
    std::shared_ptr<SignManager> Signs = Game->GetManager<SignManager>();
    std::shared_ptr<GhostManager> Ghosts = Game->GetManager<GhostManager>();

    Statistics.clear();
    Statistics["Active Players"] = std::to_string(Clients.size());
    Statistics["Unique Players"] = std::to_string(UniquePlayerCount);
    Statistics["Live Blood Messages"] = std::to_string(BloodMessages->GetLiveCount());
    Statistics["Live Blood Stains"] = std::to_string(Bloodstains->GetLiveCount());
    Statistics["Live Undead Matches"] = std::to_string(QuickMatches->GetLiveCount());
    Statistics["Live Summon Signs"] = std::to_string(Signs->GetLiveCount());
    Statistics["Live Ghosts"] = std::to_string(Ghosts->GetLiveCount());

    // Grab some populated areas stats.
    PopulatedAreas.clear();
    for (auto& Client : Clients)
    {
        PopulatedAreas[Client->GetPlayerState().GetCurrentArea()]++;
    }
}

bool StatisticsHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    {
        std::scoped_lock lock(DataMutex);

        auto activePlayerSamples = nlohmann::json::array();
        for (Sample& Value : Samples)
        {
            auto sample = nlohmann::json::object();
            sample["time"] = Value.Timestamp;
            sample["players"] = Value.ActivePlayers;
            activePlayerSamples.push_back(sample);
        }

        auto populatedAreas = nlohmann::json::array();
        for (auto& Stat : PopulatedAreas)
        {
            auto area = nlohmann::json::object();
            area["areaName"] = GetEnumString(Stat.first);
            area["playerCount"] = Stat.second;
            populatedAreas.push_back(area);
        }

        auto statistics = nlohmann::json::array();
        for (auto& Stat : Statistics)
        {
            auto stat = nlohmann::json::object();
            stat["name"] = Stat.first;
            stat["value"] = Stat.second;
            statistics.push_back(stat); 
        }

        json["activePlayerSamples"] = activePlayerSamples;
        json["populatedAreas"] = populatedAreas;
        json["statistics"] = statistics;
    }

    RespondJson(Connection, json);

    MarkAsNeedsDataGather();

    return true;
}
