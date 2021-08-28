/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Visitor/VisitorManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

VisitorManager::VisitorManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

MessageHandleResult VisitorManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetVisitorList)
    {
        return Handle_RequestGetVisitorList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestVisit)
    {
        return Handle_RequestVisit(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectVisit)
    {
        return Handle_RequestRejectVisit(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool VisitorManager::CanMatchWith(const Frpg2RequestMessage::MatchingParameter& Request, const std::shared_ptr<GameClient>& Match)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    const RuntimeConfigMatchingParameters* MatchingParams = &Config.CovenantInvasionMatchingParameters;
    if (Match->GetPlayerState().VisitorPool == Frpg2RequestMessage::VisitorPool::VisitorPool_Way_of_Blue)
    {
        MatchingParams = &Config.WayOfBlueMatchingParameters;
    }

    // Check matching parameters.
    if (!MatchingParams->CheckMatch(
            Request.soul_level(), Request.weapon_level(),
            Match->GetPlayerState().SoulLevel, Match->GetPlayerState().MaxWeaponLevel,
            Request.password().size() > 0
        ))
    {
        return false;
    }

    return true;
}

MessageHandleResult VisitorManager::Handle_RequestGetVisitorList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetVisitorList* Request = (Frpg2RequestMessage::RequestGetVisitorList*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<GameClient>> PotentialTargets = GameServiceInstance->FindClients([this, Client, Request](const std::shared_ptr<GameClient>& OtherClient) {
        if (Client == OtherClient.get())
        {
            return false;
        }
        if (Request->visitor_pool() != OtherClient->GetPlayerState().VisitorPool)
        {
            return false;
        }
        return CanMatchWith(Request->matching_parameter(), OtherClient); 
    });

    // TODO: Sort potential targets based on prioritization (more summons etc)

    Frpg2RequestMessage::RequestGetVisitorListResponse Response;
    Response.set_map_id(Request->map_id());
    Response.set_online_area_id(Request->online_area_id());

    int CountToSend = std::min((int)Request->max_visitors(), (int)PotentialTargets.size());
    for (int i = 0; i < CountToSend; i++)
    {
        std::shared_ptr<GameClient> OtherClient = PotentialTargets[i];

        Frpg2RequestMessage::VisitorData* Data = Response.add_visitors();
        Data->set_player_id(OtherClient->GetPlayerState().PlayerId);
        Data->set_player_steam_id(OtherClient->GetPlayerState().SteamId);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetVisitorListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult VisitorManager::Handle_RequestVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestVisit* Request = (Frpg2RequestMessage::RequestVisit*)Message.Protobuf.get();

    bool bSuccess = true;

    // Check client still exists.
    std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!TargetClient)
    {
        Warning("[%s] Client attempted to target unknown (or disconnected) client for visit %i.", Client->GetName().c_str(), Request->player_id());
        bSuccess = false;
    }

    // If success sent push to target client.
    if (bSuccess && TargetClient)
    {
        Frpg2RequestMessage::PushRequestVisit PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestVisit);
        PushMessage.set_player_id(Player.PlayerId);
        PushMessage.set_player_steam_id(Player.SteamId);
        PushMessage.set_data(Request->data());
        PushMessage.set_visitor_pool(Request->visitor_pool());
        PushMessage.set_map_id(Request->map_id());
        PushMessage.set_online_area_id(Request->online_area_id());

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestBreakInTarget to target of visit.", Client->GetName().c_str());
            bSuccess = false;
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    Frpg2RequestMessage::RequestVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestVisitResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    // Otherwise send rejection to client.
    if (!bSuccess)
    {
        Frpg2RequestMessage::PushRequestRejectVisit PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectVisit);
        PushMessage.set_player_id(Player.PlayerId);
        PushMessage.set_visitor_pool(Request->visitor_pool());   
        PushMessage.set_steam_id(Player.SteamId);
        PushMessage.set_unknown_5(0);
        if (TargetClient)
        {
            PushMessage.set_steam_id(TargetClient->GetPlayerState().SteamId);
        }

        if (!Client->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestRejectVisit.", Client->GetName().c_str());
            return MessageHandleResult::Error;
        }
    }
    // On success the server immediately sends a PushRequestRemoveVisitor message.
    else
    {
        Frpg2RequestMessage::PushRequestRemoveVisitor PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRemoveVisitor);
        PushMessage.set_player_id(Request->player_id());
        if (TargetClient)
        {
            PushMessage.set_player_steam_id(TargetClient->GetPlayerState().SteamId);
        }
        PushMessage.set_visitor_pool(Request->visitor_pool());

        if (!Client->MessageStream->Send(&PushMessage, &Message))
        {
            Warning("[%s] Disconnecting client as failed to send PushRequestRemoveVisitor response.", Client->GetName().c_str());
            return MessageHandleResult::Error;
        }
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult VisitorManager::Handle_RequestRejectVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestRejectVisit* Request = (Frpg2RequestMessage::RequestRejectVisit*)Message.Protobuf.get();

    // Get client who initiated the visit.
    std::shared_ptr<GameClient> InitiatorClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!InitiatorClient)
    {
        Warning("[%s] Client rejected visit from unknown (or disconnected) client %i.", Client->GetName().c_str(), Request->player_id());
        return MessageHandleResult::Handled;
    }

    // Reject the visit
    Frpg2RequestMessage::PushRequestRejectVisit PushMessage;
    PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectVisit);
    PushMessage.set_player_id(Player.PlayerId);
    PushMessage.set_visitor_pool(Request->visitor_pool());
    PushMessage.set_steam_id(Player.SteamId);
    PushMessage.set_unknown_5(0);

    if (!InitiatorClient->MessageStream->Send(&PushMessage))
    {
        Warning("[%s] Failed to send PushRequestRejectBreakInTarget to invader client %s.", Client->GetName().c_str(), InitiatorClient->GetName().c_str());
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    Frpg2RequestMessage::RequestRejectVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectVisitResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string VisitorManager::GetName()
{
    return "Visitor";
}
