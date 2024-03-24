/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Bloodstain/DS2_BloodstainManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls2/Protobuf/DS2_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS2_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS2_BloodstainManager::DS2_BloodstainManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodstainMaxLivePoolEntriesPerArea);
}

bool DS2_BloodstainManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().BloodstainPrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int StainCount = 0;

    const std::vector<DS2_OnlineAreaId>* Areas = GetEnumValues<DS2_OnlineAreaId>();
    for (DS2_OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<Bloodstain>> Stains = Database.FindRecentBloodstains((uint32_t)AreaId, PrimeCountPerArea);
        for (const std::shared_ptr<Bloodstain>& Stain : Stains)
        {
            LiveCache.Add({ Stain->CellId, AreaId }, Stain->BloodstainId, Stain);
            StainCount++;
        }
    }

    if (StainCount > 0)
    {
        LogS(GetName().c_str(), "Primed live cache with %i blood stains.", StainCount);
    }

    return true;
}

void DS2_BloodstainManager::TrimDatabase()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int MaxEntries = ServerInstance->GetConfig().BloodstainMaxDatabaseEntries;

    Database.TrimBloodStains(MaxEntries);
}

MessageHandleResult DS2_BloodstainManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestCreateBloodstain))
    {
        return Handle_RequestCreateBloodstain(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetBloodstainList))
    {
        return Handle_RequestGetBloodstainList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetAreaBloodstainList))
    {
        return Handle_RequestGetAreaBloodstainList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetDeadingGhost))
    {
        return Handle_RequestGetDeadingGhost(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_BloodstainManager::Handle_RequestCreateBloodstain(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestCreateBloodstain* Request = (DS2_Frpg2RequestMessage::RequestCreateBloodstain*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    std::vector<uint8_t> GhostData;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());
    GhostData.assign(Request->ghost_data().data(), Request->ghost_data().data() + Request->ghost_data().size());

    // There is no NRSSR struct in bloodstain or ghost data, but we still make sure the size-delimited entry list is valid.
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(Data.data(), Data.size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "Bloodstain metadata recieved from client is invalid (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            return MessageHandleResult::Handled;
        }
        ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(GhostData.data(), GhostData.size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "Ghost data recieved from client is invalid (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            return MessageHandleResult::Handled;
        }
    }

    std::shared_ptr<Bloodstain> ActiveStain = nullptr;
    if (Config.BloodstainMemoryCacheOnly)
    {
        ActiveStain = std::make_shared<Bloodstain>();
        ActiveStain->BloodstainId = (uint32_t)NextMemoryCacheStainId--;
        ActiveStain->OnlineAreaId = (uint32_t)Request->online_area_id();
        ActiveStain->CellId = (uint64_t)Request->cell_id();
        ActiveStain->PlayerId = Player.GetPlayerId();
        ActiveStain->PlayerSteamId = Player.GetSteamId();
        ActiveStain->Data = Data;
        ActiveStain->GhostData = GhostData;
    }
    else
    {
        ActiveStain = Database.CreateBloodstain(
            (uint32_t)Request->online_area_id(),
            0,
            Player.GetPlayerId(),
            Player.GetSteamId(),
            Data,
            GhostData
        );
    }

    if (ActiveStain)
    {
        LiveCache.Add({ ActiveStain->CellId, (DS2_OnlineAreaId)ActiveStain->OnlineAreaId }, ActiveStain->BloodstainId, ActiveStain);
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to create blood stain.");
    }

    std::string TypeStatisticKey = StringFormat("Bloodstain/TotalCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodstainManager::Handle_RequestGetBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetBloodstainList* Request = (DS2_Frpg2RequestMessage::RequestGetBloodstainList*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetBloodstainListResponse Response;
    Response.mutable_bloodstains();

    int RemainingStainCount = (int)Request->max_stains();

    if (!Config.DisableBloodStains)
    {
        // Grab a random set of stains from the live cache.
        for (int i = 0; i < Request->search_areas_size() && RemainingStainCount > 0; i++)
        {
            const DS2_Frpg2RequestMessage::CellLimitData& Area = Request->search_areas(i);

            uint64_t CellId = Area.cell_id();
            int MaxForCell = (int)Area.max_items();
            int GatherCount = std::min(MaxForCell, RemainingStainCount);
            
            std::vector<std::shared_ptr<Bloodstain>> AreaStains = LiveCache.GetRandomSet(GatherCount, [CellId](DS2_CellAndAreaId Id) {
                return Id.CellId == CellId;
            });
            for (std::shared_ptr<Bloodstain>& AreaMsg : AreaStains)
            {
                DS2_Frpg2RequestMessage::BloodstainInfo& Data = *Response.mutable_bloodstains()->Add();
                Data.set_online_area_id((uint32_t)AreaMsg->OnlineAreaId);
                Data.set_cell_id(AreaMsg->CellId);
                Data.set_bloodstain_id((uint32_t)AreaMsg->BloodstainId);
                Data.set_data(AreaMsg->Data.data(), AreaMsg->Data.size());
            
                RemainingStainCount--;
            }
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCreateBloodMessageResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodstainManager::Handle_RequestGetAreaBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetAreaBloodstainList* Request = (DS2_Frpg2RequestMessage::RequestGetAreaBloodstainList*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetBloodstainListResponse Response;
    Response.mutable_bloodstains();

    if (!Config.DisableBloodStains)
    {
        DS2_OnlineAreaId AreaId = (DS2_OnlineAreaId)Request->online_area_id();
        int MaxForArea = Request->count();

        std::vector<std::shared_ptr<Bloodstain>> AreaStains = LiveCache.GetRandomSet(MaxForArea, [AreaId](DS2_CellAndAreaId Id) {
            return Id.AreaId == AreaId;
        });

        for (std::shared_ptr<Bloodstain>& AreaMsg : AreaStains)
        {
            DS2_Frpg2RequestMessage::BloodstainInfo& Data = *Response.mutable_bloodstains()->Add();
            Data.set_online_area_id(AreaMsg->OnlineAreaId);
            Data.set_cell_id(AreaMsg->CellId);
            Data.set_bloodstain_id(AreaMsg->BloodstainId);
            Data.set_data(AreaMsg->Data.data(), AreaMsg->Data.size());
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetAreaBloodMessageListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BloodstainManager::Handle_RequestGetDeadingGhost(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetDeadingGhost* Request = (DS2_Frpg2RequestMessage::RequestGetDeadingGhost*)Message.Protobuf.get();

    std::shared_ptr<Bloodstain> ActiveStain;

    DS2_CellAndAreaId LocationId = { Request->cell_id(), (DS2_OnlineAreaId)Request->online_area_id() };

    // Find it in live cache.
    if (ActiveStain = LiveCache.Find(LocationId, Request->bloodstain_id()))
    {
        // Nothing needed here.
    }
    // If not in cache, grab it from database.
    else if (ActiveStain =  Database.FindBloodstain(Request->bloodstain_id()))
    {
        LiveCache.Add(LocationId, ActiveStain->BloodstainId, ActiveStain);
    }
    // Doesn't exist, no go.
    else
    {
        WarningS(Client->GetName().c_str(), "Failed to retrieve bloodstain '%u'", Request->bloodstain_id());
    }

    DS2_Frpg2RequestMessage::RequestGetDeadingGhostResponse Response;
    Response.set_online_area_id(Request->online_area_id());
    Response.set_cell_id(Request->cell_id());
    Response.set_bloodstain_id(Request->bloodstain_id());

    if (ActiveStain == nullptr)
    {
        Response.mutable_data();
    }
    else
    {
        Response.set_data(ActiveStain->GhostData.data(), ActiveStain->GhostData.size());
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetDeadingGhostResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_BloodstainManager::GetName()
{
    return "Bloodstain";
}
