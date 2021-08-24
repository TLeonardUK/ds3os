/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ghosts/GhostManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

GhostManager::GhostManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
    LiveCache.SetMaxEntriesPerArea(InServerInstance->GetConfig().GhostMaxLivePoolEntriesPerArea);
}

bool GhostManager::Init()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int PrimeCountPerArea = ServerInstance->GetConfig().GhostPrimeCountPerArea;

    // Prime the cache with a handful of the most recent messages from the database.
    int GhostCount = 0;

    const std::vector<OnlineAreaId>* Areas = GetEnumValues<OnlineAreaId>();
    for (OnlineAreaId AreaId : *Areas)
    {
        std::vector<std::shared_ptr<Ghost>> Ghosts = Database.FindRecentGhosts(AreaId, PrimeCountPerArea);
        for (const std::shared_ptr<Ghost>& Ghost : Ghosts)
        {
            LiveCache.Add(AreaId, Ghost->GhostId, Ghost);
            GhostCount++;
        }
    }

    if (GhostCount > 0)
    {
        Log("[%s] Primed live cache with %i ghosts.", GetName().c_str(), GhostCount);
    }

    return true;
}

MessageHandleResult GhostManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateGhostData)
    {
        return Handle_RequestCreateGhostData(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetGhostDataList)
    {
        return Handle_RequestGetGhostDataList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult GhostManager::Handle_RequestCreateGhostData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestCreateGhostData* Request = (Frpg2RequestMessage::RequestCreateGhostData*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());

    if (std::shared_ptr<Ghost> ActiveGhost = Database.CreateGhost((OnlineAreaId)Request->online_area_id(), Player.PlayerId, Player.SteamId, Data))
    {
        LiveCache.Add(ActiveGhost->OnlineAreaId, ActiveGhost->GhostId, ActiveGhost);
    }
    else
    {
        Warning("[%s] Disconnecting client as failed to create ghost.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    Frpg2RequestMessage::RequestCreateGhostDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateGhostDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult GhostManager::Handle_RequestGetGhostDataList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetGhostDataList* Request = (Frpg2RequestMessage::RequestGetGhostDataList*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestGetGhostDataListResponse Response;

    uint32_t RemainingGhostCount = Request->max_ghosts();

    // Grab a random set of stains from the live cache.
    for (int i = 0; i < Request->search_areas_size() && RemainingGhostCount > 0; i++)
    {
        const Frpg2RequestMessage::DomainLimitData& Area = Request->search_areas(i);

        OnlineAreaId AreaId = (OnlineAreaId)Area.online_area_id();
        uint32_t MaxForArea = Area.max_items();
        uint32_t GatherCount = std::min(MaxForArea, RemainingGhostCount);

        std::vector<std::shared_ptr<Ghost>> ActiveGhosts = LiveCache.GetRandomSet(AreaId, GatherCount);
        for (std::shared_ptr<Ghost>& AreaMsg : ActiveGhosts)
        {
            // Filter players own messages.
            if (AreaMsg->PlayerId == Player.PlayerId)
            {
                continue;
            }

            //Log("[%s] Returning ghost %i in area %i.", Client->GetName().c_str(), AreaMsg->GhostId, AreaMsg->OnlineAreaId);

            Frpg2RequestMessage::GhostData& Data = *Response.mutable_ghosts()->Add();
            Data.set_unknown_1(1);                                                      // TODO: Figure out what this is.
            Data.set_ghost_id((uint32_t)AreaMsg->GhostId);
            Data.set_data(AreaMsg->Data.data(), AreaMsg->Data.size());

            RemainingGhostCount--;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetGhostDataListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string GhostManager::GetName()
{
    return "Ghosts";
}
