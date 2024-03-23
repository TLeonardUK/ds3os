/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ghosts/DS3_GhostManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS3_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS3_GhostManager::DS3_GhostManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().GhostMaxLivePoolEntriesPerArea);
}

bool DS3_GhostManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().GhostPrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int GhostCount = 0;

    const std::vector<DS3_OnlineAreaId>* Areas = GetEnumValues<DS3_OnlineAreaId>();
    for (DS3_OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<Ghost>> Ghosts = Database.FindRecentGhosts((uint32_t)AreaId, 0, PrimeCountPerArea);
        for (const std::shared_ptr<Ghost>& Ghost : Ghosts)
        {
            LiveCache.Add(AreaId, Ghost->GhostId, Ghost);
            GhostCount++;
        }
    }

    if (GhostCount > 0)
    {
        LogS(GetName().c_str(), "Primed live cache with %i ghosts.", GhostCount);
    }

    return true;
}

void DS3_GhostManager::TrimDatabase()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int MaxEntries = ServerInstance->GetConfig().GhostMaxDatabaseEntries;

    Database.TrimGhosts(MaxEntries);
}

MessageHandleResult DS3_GhostManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestCreateGhostData))
    {
        return Handle_RequestCreateGhostData(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetGhostDataList))
    {
        return Handle_RequestGetGhostDataList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_GhostManager::Handle_RequestCreateGhostData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestCreateGhostData* Request = (DS3_Frpg2RequestMessage::RequestCreateGhostData*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());
    
    // There is no NRSSR struct in ghost data, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Data.data(), Data.size());
        if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "Ghost data recieved from client is invalid (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            return MessageHandleResult::Handled;
        }
    }

    std::shared_ptr<Ghost> ActiveGhost = nullptr;
    if (Config.GhostMemoryCacheOnly)
    {
        ActiveGhost = std::make_shared<Ghost>();
        ActiveGhost->GhostId = (uint32_t)NextMemoryCacheGhostId--;
        ActiveGhost->OnlineAreaId = (uint32_t)Request->online_area_id();
        ActiveGhost->CellId = 0;
        ActiveGhost->PlayerId = Player.GetPlayerId();
        ActiveGhost->PlayerSteamId = Player.GetSteamId();
        ActiveGhost->Data = Data;
    }
    else
    {
        ActiveGhost = Database.CreateGhost(
            (uint32_t)Request->online_area_id(),
            0,
            Player.GetPlayerId(),
            Player.GetSteamId(),
            Data);
    }

    if (ActiveGhost)
    {
        LiveCache.Add((DS3_OnlineAreaId)ActiveGhost->OnlineAreaId, ActiveGhost->GhostId, ActiveGhost);
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to create ghost.");
    }

    std::string TypeStatisticKey = StringFormat("Ghosts/TotalGhostsCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS3_Frpg2RequestMessage::RequestCreateGhostDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCreateGhostDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_GhostManager::Handle_RequestGetGhostDataList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS3_Frpg2RequestMessage::RequestGetGhostDataList* Request = (DS3_Frpg2RequestMessage::RequestGetGhostDataList*)Message.Protobuf.get();
    DS3_Frpg2RequestMessage::RequestGetGhostDataListResponse Response;
    Response.mutable_ghosts();

    uint32_t RemainingGhostCount = Request->max_ghosts();

    if (!Config.DisableGhosts)
    {
        // Grab a random set of stains from the live cache.
        for (int i = 0; i < Request->search_areas_size() && RemainingGhostCount > 0; i++)
        {
            const DS3_Frpg2RequestMessage::DomainLimitData& Area = Request->search_areas(i);

            DS3_OnlineAreaId AreaId = (DS3_OnlineAreaId)Area.online_area_id();
            uint32_t MaxForArea = Area.max_items();
            uint32_t GatherCount = std::min(MaxForArea, RemainingGhostCount);

            std::vector<std::shared_ptr<Ghost>> ActiveGhosts = LiveCache.GetRandomSet(AreaId, GatherCount);
            for (std::shared_ptr<Ghost>& AreaMsg : ActiveGhosts)
            {
                DS3_Frpg2RequestMessage::GhostData& Data = *Response.mutable_ghosts()->Add();
                Data.set_unknown_1(1);                                                      // TODO: Figure out what this is.
                Data.set_ghost_id((uint32_t)AreaMsg->GhostId);
                Data.set_data(AreaMsg->Data.data(), AreaMsg->Data.size());

                RemainingGhostCount--;
            }
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetGhostDataListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS3_GhostManager::GetName()
{
    return "Ghosts";
}
