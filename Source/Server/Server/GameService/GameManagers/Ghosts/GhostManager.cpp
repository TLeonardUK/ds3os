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

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

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
        LogS(GetName().c_str(), "Primed live cache with %i ghosts.", GhostCount);
    }

    return true;
}

void GhostManager::TrimDatabase()
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    int MaxEntries = ServerInstance->GetConfig().GhostMaxDatabaseEntries;

    Database.TrimGhosts(MaxEntries);
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

    DS3_Frpg2RequestMessage::RequestCreateGhostData* Request = (DS3_Frpg2RequestMessage::RequestCreateGhostData*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->data().data(), Request->data().data() + Request->data().size());

    if (std::shared_ptr<Ghost> ActiveGhost = Database.CreateGhost((OnlineAreaId)Request->online_area_id(), Player.GetPlayerId(), Player.GetSteamId(), Data))
    {
        LiveCache.Add(ActiveGhost->OnlineAreaId, ActiveGhost->GhostId, ActiveGhost);
    }
    else
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to create ghost.");
        return MessageHandleResult::Error;
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

MessageHandleResult GhostManager::Handle_RequestGetGhostDataList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
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

            OnlineAreaId AreaId = (OnlineAreaId)Area.online_area_id();
            uint32_t MaxForArea = Area.max_items();
            uint32_t GatherCount = std::min(MaxForArea, RemainingGhostCount);

            std::vector<std::shared_ptr<Ghost>> ActiveGhosts = LiveCache.GetRandomSet(AreaId, GatherCount);
            for (std::shared_ptr<Ghost>& AreaMsg : ActiveGhosts)
            {
                // Filter players own messages.
                if (AreaMsg->PlayerId == Player.GetPlayerId())
                {
                    continue;
                }

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

std::string GhostManager::GetName()
{
    return "Ghosts";
}
