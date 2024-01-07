/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BreakIn/DS2_BreakInManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"
#include "Server/GameService/Utils/DS2_GameIds.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

DS2_BreakInManager::DS2_BreakInManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

void DS2_BreakInManager::OnLostPlayer(GameClient* Client)
{
}

MessageHandleResult DS2_BreakInManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetBreakInTargetList))
    {
        return Handle_RequestGetBreakInTargetList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestBreakInTarget))
    {
        return Handle_RequestBreakInTarget(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRejectBreakInTarget))
    {
        return Handle_RequestRejectBreakInTarget(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool DS2_BreakInManager::CanMatchWith(const DS2_Frpg2RequestMessage::MatchingParameter& Request, const std::shared_ptr<GameClient>& Match, DS2_Frpg2RequestMessage::BreakInType Type)
{
    auto& Player = Match->GetPlayerStateType<DS2_PlayerState>();
    const RuntimeConfig& Config = ServerInstance->GetConfig();

    // Invasions disabled.
    if (Config.DisableInvasions)
    {
        return false;
    }

    if (!Match->GetPlayerState().GetIsInvadable())
    {
        return false;
    }

    // If doing a blue eye invasion, make sure the match is a sinner.
    if (Type == DS2_Frpg2RequestMessage::BreakInType_BlueEyeOrb)
    {
        if (!Player.GetPlayerStatus().has_stats_info() || Player.GetPlayerStatus().stats_info().sinner_points() < 10)
        {
            return false;
        }
    }

    // Add matchmaking
    switch (Type)
    {
        case DS2_Frpg2RequestMessage::BreakInType_RedEyeOrb:
        {
            return Config.DS2_RedEyeOrbMatchingParameters.CheckMatch(Player.GetSoulMemory(), Request.soul_memory(), false);
        }
        case DS2_Frpg2RequestMessage::BreakInType_BlueEyeOrb:
        {
            return Config.DS2_BlueEyeOrbMatchingParameters.CheckMatch(Player.GetSoulMemory(), Request.soul_memory(), false);
        }
    }

    return true;
}

MessageHandleResult DS2_BreakInManager::Handle_RequestGetBreakInTargetList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    auto& Player = Client->GetPlayerStateType<DS2_PlayerState>();

    const RuntimeConfig& Config = ServerInstance->GetConfig();

    DS2_Frpg2RequestMessage::RequestGetBreakInTargetList* Request = (DS2_Frpg2RequestMessage::RequestGetBreakInTargetList*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<GameClient>> PotentialTargets = GameServiceInstance->FindClients([this, Client, Request, Config, Player](const std::shared_ptr<GameClient>& OtherClient) {
        if (Client == OtherClient.get())
        {
            return false;
        }
        if (Config.IgnoreInvasionAreaFilter)
        {          
            bool InValidArea = false;
            for (size_t i = 0; i < Player.GetPlayerStatus().player_status().played_areas_size(); i++)
            {
                DS2_OnlineAreaId AreaId = (DS2_OnlineAreaId)Player.GetPlayerStatus().player_status().played_areas((int)i);
                if (OtherClient->GetPlayerStateType<DS2_PlayerState>().GetCurrentArea() == AreaId)
                {
                    InValidArea = true;
                    break;
                }
            }
                
            if (!InValidArea)
            {
                return false;
            }
        }
        else
        {
            if (OtherClient->GetPlayerStateType<DS2_PlayerState>().GetCurrentArea() != (DS2_OnlineAreaId)Request->online_area_id())
            {
                return false;
            }
        }
        return CanMatchWith(Request->matching_parameter(), OtherClient, Request->type()); 
    });

    DS2_Frpg2RequestMessage::RequestGetBreakInTargetListResponse Response;
    Response.set_cell_id(Request->cell_id());
    Response.set_online_area_id(Request->online_area_id());

    int CountToSend = std::min((int)Request->max_targets(), (int)PotentialTargets.size());
    for (int i = 0; i < CountToSend; i++)
    {
        std::shared_ptr<GameClient> OtherClient = PotentialTargets[i];

        DS2_Frpg2RequestMessage::BreakInTargetData* Data = Response.add_target_data();
        Data->set_player_id(OtherClient->GetPlayerState().GetPlayerId());
        Data->set_steam_id(OtherClient->GetPlayerState().GetSteamId());
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetBreakInTargetListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BreakInManager::Handle_RequestBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestBreakInTarget* Request = (DS2_Frpg2RequestMessage::RequestBreakInTarget*)Message.Protobuf.get();

    bool bSuccess = true;

    // Check client still exists.
    std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!TargetClient)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to target unknown (or disconnected) client for invasion %i.", Request->player_id());
        bSuccess = false;
    }

    // If success sent push to target client.
    if (bSuccess && TargetClient)
    {
        DS2_Frpg2RequestMessage::PushRequestBreakInTarget PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestBreakInTarget);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_steam_id(Player.GetSteamId());
        PushMessage.set_type(Request->type());
        PushMessage.set_cell_id(Request->cell_id());
        PushMessage.set_online_area_id(Request->online_area_id());

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestBreakInTarget to target of invasion.");
            bSuccess = false;
        }

        std::string TypeStatisticKey = StringFormat("BreakIn/TotalInvasionsRequested");
        Database.AddGlobalStatistic(TypeStatisticKey, 1);
        Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestBreakInTargetResponse response.");
        return MessageHandleResult::Error;
    }

    // Otherwise send rejection to client.
    if (!bSuccess)
    {
        DS2_Frpg2RequestMessage::PushRequestRejectBreakInTarget PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectBreakInTarget);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_unknown_3(1);
        PushMessage.set_steam_id(Player.GetSteamId());
        PushMessage.set_unknown_5(0);
        if (TargetClient)
        {
            PushMessage.set_player_id(TargetClient->GetPlayerState().GetPlayerId());
            PushMessage.set_steam_id(TargetClient->GetPlayerState().GetSteamId());
        }

        if (!Client->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectBreakInTarget.");
            return MessageHandleResult::Error;
        }
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_BreakInManager::Handle_RequestRejectBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestRejectBreakInTarget* Request = (DS2_Frpg2RequestMessage::RequestRejectBreakInTarget*)Message.Protobuf.get();

    // Get client who initiated the invasion.
    std::shared_ptr<GameClient> InvaderClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!InvaderClient)
    {
        WarningS(Client->GetName().c_str(), "Client rejected breakin from unknown (or disconnected) client %i.", Request->player_id());
        return MessageHandleResult::Handled;
    }

    // Reject the break in.
    DS2_Frpg2RequestMessage::PushRequestRejectBreakInTarget PushMessage;
    PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectBreakInTarget);
    PushMessage.set_player_id(Player.GetPlayerId());
    PushMessage.set_unknown_3(1);
    PushMessage.set_steam_id(Player.GetSteamId());
    PushMessage.set_unknown_5(0);

    if (!InvaderClient->MessageStream->Send(&PushMessage))
    {
        WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectBreakInTarget to invader client %s.", InvaderClient->GetName().c_str());
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestRejectBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectBreakInTargetResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_BreakInManager::GetName()
{
    return "Break In";
}
