/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Bloodstain/BloodstainManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

BloodstainManager::BloodstainManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().BloodstainMaxLivePoolEntriesPerArea);
}

bool BloodstainManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().BloodstainPrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int StainCount = 0;

    const std::vector<OnlineAreaId>* Areas = GetEnumValues<OnlineAreaId>();
    for (OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<Bloodstain>> Stains = Database.FindRecentBloodstains(AreaId, PrimeCountPerArea);
        for (const std::shared_ptr<Bloodstain>& Stain : Stains)
        {
            LiveCache.Add(AreaId, Stain->BloodstainId, Stain);
            StainCount++;
        }
    }

    if (StainCount > 0)
    {
        Log("[%s] Primed live cache with %i blood stains.", GetName().c_str(), StainCount);
    }

    return true;
}

MessageHandleResult BloodstainManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateBloodstain)
    {
        return Handle_RequestCreateBloodstain(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBloodstainList)
    {
        return Handle_RequestGetBloodstainList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetDeadingGhost)
    {
        return Handle_RequestGetDeadingGhost(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult BloodstainManager::Handle_RequestCreateBloodstain(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestCreateBloodstain* Request = (Frpg2RequestMessage::RequestCreateBloodstain*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    std::vector<uint8_t> GhostData;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());
    GhostData.assign(Request->ghost_data().data(), Request->ghost_data().data() + Request->ghost_data().size());

    if (std::shared_ptr<Bloodstain> ActiveStain = Database.CreateBloodstain((OnlineAreaId)Request->online_area_id(), Player.PlayerId, Player.SteamId, Data, GhostData))
    {
        LiveCache.Add(ActiveStain->OnlineAreaId, ActiveStain->BloodstainId, ActiveStain);
    }
    else
    {
        Warning("[%s] Disconnecting client as failed to create blood stain.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    std::string TypeStatisticKey = StringFormat("Bloodstain/TotalCreated");
    Database.AddGlobalStatistic(TypeStatisticKey, 1);
    Database.AddPlayerStatistic(TypeStatisticKey, Player.PlayerId, 1);

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodstainManager::Handle_RequestGetBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetBloodstainList* Request = (Frpg2RequestMessage::RequestGetBloodstainList*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestGetBloodstainListResponse Response;

    uint32_t RemainingStainCount = Request->max_stains();

    // Grab a random set of stains from the live cache.
    for (int i = 0; i < Request->search_areas_size() && RemainingStainCount > 0; i++)
    {
        const Frpg2RequestMessage::DomainLimitData& Area = Request->search_areas(i);

        OnlineAreaId AreaId = (OnlineAreaId)Area.online_area_id();
        uint32_t MaxForArea = Area.max_items();
        uint32_t GatherCount = std::min(MaxForArea, RemainingStainCount);

        std::vector<std::shared_ptr<Bloodstain>> AreaStains = LiveCache.GetRandomSet(AreaId, GatherCount);
        for (std::shared_ptr<Bloodstain>& AreaMsg : AreaStains)
        {
            // Filter players own messages.
            if (AreaMsg->PlayerId == Player.PlayerId)
            {
                continue;
            }

            //Log("[%s] Returning blood stain %i in area %i.", Client->GetName().c_str(), AreaMsg->BloodstainId, AreaMsg->OnlineAreaId);

            Frpg2RequestMessage::BloodstainInfo& Data = *Response.mutable_bloodstains()->Add();
            Data.set_online_area_id((uint32_t)AreaMsg->OnlineAreaId);
            Data.set_bloodstain_id((uint32_t)AreaMsg->BloodstainId);
            Data.set_data(AreaMsg->Data.data(), AreaMsg->Data.size());
            
            RemainingStainCount--;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}


MessageHandleResult BloodstainManager::Handle_RequestGetDeadingGhost(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetDeadingGhost* Request = (Frpg2RequestMessage::RequestGetDeadingGhost*)Message.Protobuf.get();

    std::shared_ptr<Bloodstain> ActiveStain;

    // Find it in live cache.
    if (ActiveStain = LiveCache.Find((OnlineAreaId)Request->online_area_id(), Request->bloodstain_id()))
    {
        // Nothing needed here.
    }
    // If not in cache, grab it from database.
    else if (ActiveStain =  Database.FindBloodstain(Request->bloodstain_id()))
    {
        LiveCache.Add(ActiveStain->OnlineAreaId, ActiveStain->BloodstainId, ActiveStain);
    }
    // Doesn't exist, no go.
    else
    {
        Warning("[%s] Disconnecting client as failed to retrieve bloodstain '%i'", Client->GetName().c_str(), Request->bloodstain_id());
        return MessageHandleResult::Error;
    }

    Frpg2RequestMessage::RequestGetDeadingGhostResponse Response;
    Response.set_online_area_id(Request->online_area_id());
    Response.set_bloodstain_id(Request->bloodstain_id());
    Response.set_data(ActiveStain->GhostData.data(), ActiveStain->GhostData.size());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetDeadingGhostResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BloodstainManager::GetName()
{
    return "Bloodstain";
}
