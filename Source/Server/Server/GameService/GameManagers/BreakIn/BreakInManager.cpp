/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BreakIn/BreakInManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

BreakInManager::BreakInManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

void BreakInManager::OnLostPlayer(GameClient* Client)
{
}

MessageHandleResult BreakInManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBreakInTargetList)
    {
        return Handle_RequestGetBreakInTargetList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestBreakInTarget)
    {
        return Handle_RequestBreakInTarget(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectBreakInTarget)
    {
        return Handle_RequestRejectBreakInTarget(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool BreakInManager::CanMatchWith(const Frpg2RequestMessage::MatchingParameter& Request, const std::shared_ptr<GameClient>& Match)
{
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

    const RuntimeConfigMatchingParameters* MatchingParams = &Config.DarkSpiritInvasionMatchingParameters;
    if (Request.covenant() == Frpg2RequestMessage::Covenant::Covenant_Mound_Makers)
    {
        MatchingParams = &Config.MoundMakerInvasionMatchingParameters;
    }

    // Check matching parameters.
    if (!MatchingParams->CheckMatch(
            Request.soul_level(), Request.weapon_level(), 
            Match->GetPlayerState().GetSoulLevel(), Match->GetPlayerState().GetMaxWeaponLevel(),
            Request.password().size() > 0
        ))
    {
        return false;
    }

    return true;
}

MessageHandleResult BreakInManager::Handle_RequestGetBreakInTargetList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetBreakInTargetList* Request = (Frpg2RequestMessage::RequestGetBreakInTargetList*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<GameClient>> PotentialTargets = GameServiceInstance->FindClients([this, Client, Request](const std::shared_ptr<GameClient>& OtherClient) {
        if (Client == OtherClient.get())
        {
            return false;
        }
        if (OtherClient->GetPlayerState().GetCurrentArea() != (OnlineAreaId)Request->online_area_id())
        {
            return false;
        }
        return CanMatchWith(Request->matching_parameter(), OtherClient); 
    });

    // TODO: Sort potential targets based on prioritization (more summons etc)

    Frpg2RequestMessage::RequestGetBreakInTargetListResponse Response;
    Response.set_map_id(Request->map_id());
    Response.set_online_area_id(Request->online_area_id());

    int CountToSend = std::min((int)Request->max_targets(), (int)PotentialTargets.size());
    for (int i = 0; i < CountToSend; i++)
    {
        std::shared_ptr<GameClient> OtherClient = PotentialTargets[i];

        Frpg2RequestMessage::BreakInTargetData* Data = Response.add_target_data();
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

MessageHandleResult BreakInManager::Handle_RequestBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestBreakInTarget* Request = (Frpg2RequestMessage::RequestBreakInTarget*)Message.Protobuf.get();

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
        Frpg2RequestMessage::PushRequestBreakInTarget PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestBreakInTarget);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_steam_id(Player.GetSteamId());
        PushMessage.set_unknown_4(0);
        PushMessage.set_map_id(Request->map_id());
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
    Frpg2RequestMessage::RequestBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestBreakInTargetResponse response.");
        return MessageHandleResult::Error;
    }

    // Otherwise send rejection to client.
    if (!bSuccess)
    {
        Frpg2RequestMessage::PushRequestRejectBreakInTarget PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectBreakInTarget);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_unknown_3(1);     // TODO: Figure out
        PushMessage.set_steam_id(Player.GetSteamId());
        PushMessage.set_unknown_5(0);
        if (TargetClient)
        {
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

MessageHandleResult BreakInManager::Handle_RequestRejectBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestRejectBreakInTarget* Request = (Frpg2RequestMessage::RequestRejectBreakInTarget*)Message.Protobuf.get();

    // Get client who initiated the invasion.
    std::shared_ptr<GameClient> InvaderClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!InvaderClient)
    {
        WarningS(Client->GetName().c_str(), "Client rejected breakin from unknown (or disconnected) client %i.", Request->player_id());
        return MessageHandleResult::Handled;
    }

    // Reject the break in.
    Frpg2RequestMessage::PushRequestRejectBreakInTarget PushMessage;
    PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectBreakInTarget);
    PushMessage.set_player_id(Player.GetPlayerId());
    PushMessage.set_unknown_3(1);     // TODO: Figure out
    PushMessage.set_steam_id(Player.GetSteamId());
    PushMessage.set_unknown_5(0);

    if (!InvaderClient->MessageStream->Send(&PushMessage))
    {
        WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectBreakInTarget to invader client %s.", InvaderClient->GetName().c_str());
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    Frpg2RequestMessage::RequestRejectBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectBreakInTargetResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BreakInManager::GetName()
{
    return "Break In";
}
