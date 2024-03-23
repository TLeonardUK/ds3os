/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Visitor/DS2_VisitorManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"
#include "Server/GameService/Utils/DS2_NRSSRSanitizer.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

DS2_VisitorManager::DS2_VisitorManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

MessageHandleResult DS2_VisitorManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetVisitorList))
    {
        return Handle_RequestGetVisitorList(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestVisit))
    {
        return Handle_RequestVisit(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRejectVisit))
    {
        return Handle_RequestRejectVisit(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool DS2_VisitorManager::CanMatchWith(const DS2_Frpg2RequestMessage::MatchingParameter& Request, const std::shared_ptr<GameClient>& Match)
{
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    auto& Player = Match->GetPlayerStateType<DS2_PlayerState>();
    bool IsInvasion = (Player.GetVisitorPool() != DS2_Frpg2RequestMessage::VisitorType::VisitorType_BlueSentinels);

    const RuntimeConfigMatchingParameters* MatchingParams = &Config.CovenantInvasionMatchingParameters;
    if (!IsInvasion)
    {
        MatchingParams = &Config.WayOfBlueMatchingParameters;
    }

    // Matching globally disabled?
    bool IsDisabled = IsInvasion ? Config.DisableInvasionAutoSummon : Config.DisableCoopAutoSummon;
    if (IsDisabled)
    {
        return false;
    }

    switch (Player.GetVisitorPool())
    {
        case DS2_Frpg2RequestMessage::VisitorType_BlueSentinels:
        {
            return Config.DS2_BlueSentinelMatchingParameters.CheckMatch(Request.soul_memory(), Player.GetSoulMemory(), false);
        }
        case DS2_Frpg2RequestMessage::VisitorType_BellKeepers:
        {
            return Config.DS2_BellKeeperMatchingParameters.CheckMatch(Request.soul_memory(), Player.GetSoulMemory(), false);
        }
        case DS2_Frpg2RequestMessage::VisitorType_Rat:
        {
            return Config.DS2_RatMatchingParameters.CheckMatch(Request.soul_memory(), Player.GetSoulMemory(), false);
        }
    }

    return true;
}

MessageHandleResult DS2_VisitorManager::Handle_RequestGetVisitorList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestGetVisitorList* Request = (DS2_Frpg2RequestMessage::RequestGetVisitorList*)Message.Protobuf.get();
    
    std::vector<std::shared_ptr<GameClient>> PotentialTargets = GameServiceInstance->FindClients([this, Client, Request](const std::shared_ptr<GameClient>& OtherClient) {
        if (Client == OtherClient.get())
        {
            return false;
        }
        if (Request->type() != OtherClient->GetPlayerStateType<DS2_PlayerState>().GetVisitorPool())
        {
            return false;
        }
        return CanMatchWith(Request->matching_parameter(), OtherClient);         
    });

    // TODO: Sort potential targets based on prioritization (more summons etc)

    DS2_Frpg2RequestMessage::RequestGetVisitorListResponse Response;
    Response.set_online_area_id(Request->online_area_id());
    Response.set_cell_id(Request->cell_id());

    int CountToSend = std::min((int)Request->max_targets(), (int)PotentialTargets.size());
    for (int i = 0; i < CountToSend; i++)
    {
        std::shared_ptr<GameClient> OtherClient = PotentialTargets[i];

        DS2_Frpg2RequestMessage::VisitorData* Data = Response.add_target_data();
        Data->set_player_id(OtherClient->GetPlayerState().GetPlayerId());
        Data->set_player_steam_id(OtherClient->GetPlayerState().GetSteamId());
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetVisitorListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_VisitorManager::Handle_RequestVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestVisit* Request = (DS2_Frpg2RequestMessage::RequestVisit*)Message.Protobuf.get();

    bool bSuccess = true;
    
    // Make sure the NRSSR data contained within this message is valid (if the CVE-2022-24126 fix is enabled)
    if (BuildConfig::NRSSR_SANITY_CHECKS)
    {
        auto ValidationResult = DS2_NRSSRSanitizer::ValidateEntryList(Request->player_struct().data(), Request->player_struct().size());
        if (ValidationResult != DS2_NRSSRSanitizer::ValidationResult::Valid)
        {
            WarningS(Client->GetName().c_str(), "RequestVisit message recieved from client contains ill formated binary data (error code %i).",
                static_cast<uint32_t>(ValidationResult));

            bSuccess = false;
        }
    }

    // Check client still exists.
    std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!TargetClient)
    {
        WarningS(Client->GetName().c_str(), "Client attempted to target unknown (or disconnected) client for visit %i.", Request->player_id());
        bSuccess = false;
    }

    // If success sent push to target client.
    if (bSuccess && TargetClient)
    {
        DS2_Frpg2RequestMessage::PushRequestVisit PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestVisit);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_player_steam_id(Player.GetSteamId());
        PushMessage.set_player_struct(Request->player_struct().c_str(), Request->player_struct().size());
        PushMessage.set_type(Request->type());
        PushMessage.set_cell_id(Request->cell_id());
        PushMessage.set_online_area_id(Request->online_area_id());

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            WarningS(TargetClient->GetName().c_str(), "Failed to send PushRequestBreakInTarget to target of visit.");
            bSuccess = false;
        }
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestVisitResponse response.");
        return MessageHandleResult::Error;
    }

    // Otherwise send rejection to client.
    if (!bSuccess)
    {
#ifdef _DEBUG
        LogS(Client->GetName().c_str(), "Sending push rejecting visit of player, as could not be found.");
#endif

        DS2_Frpg2RequestMessage::PushRequestRejectVisit PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectVisit);
        PushMessage.set_player_id(Request->player_id());
        PushMessage.set_steam_id("");
        PushMessage.set_type(Request->type());
        PushMessage.set_unknown_3(0);
        if (TargetClient)
        {
            PushMessage.set_steam_id(TargetClient->GetPlayerState().GetSteamId());
        }

        if (!Client->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectVisit.");
            return MessageHandleResult::Error;
        }
    }
    // On success the server immediately sends a PushRequestRemoveVisitor message.
    else
    {
        DS2_Frpg2RequestMessage::PushRequestRemoveVisitor PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRemoveVisitor);
        PushMessage.set_player_id(Request->player_id());
        if (TargetClient)
        {
            PushMessage.set_player_steam_id(TargetClient->GetPlayerState().GetSteamId());
        }
        PushMessage.set_type(Request->type());

        if (!Client->MessageStream->Send(&PushMessage, &Message))
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send PushRequestRemoveVisitor response.");
            return MessageHandleResult::Error;
        }

        std::string PoolStatisticKey = StringFormat("Visitor/TotalVisitsRequested/Pool=%u", (uint32_t)Request->type());
        Database.AddGlobalStatistic(PoolStatisticKey, 1);
        Database.AddPlayerStatistic(PoolStatisticKey, Player.GetPlayerId(), 1);

        std::string TypeStatisticKey = StringFormat("Visitor/TotalVisitsRequested");
        Database.AddGlobalStatistic(TypeStatisticKey, 1);
        Database.AddPlayerStatistic(TypeStatisticKey, Player.GetPlayerId(), 1);
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_VisitorManager::Handle_RequestRejectVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();
    
    DS2_Frpg2RequestMessage::RequestRejectVisit* Request = (DS2_Frpg2RequestMessage::RequestRejectVisit*)Message.Protobuf.get();

    // Get client who initiated the visit.
    std::shared_ptr<GameClient> InitiatorClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (!InitiatorClient)
    {
        WarningS(Client->GetName().c_str(), "Client rejected visit from unknown (or disconnected) client %i.", Request->player_id());
        return MessageHandleResult::Handled;
    }

    // Reject the visit
    DS2_Frpg2RequestMessage::PushRequestRejectVisit PushMessage;
    PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectVisit);
    PushMessage.set_player_id(Player.GetPlayerId());
    if (Request->has_type())
    {
        PushMessage.set_type(Request->type());
    }
    PushMessage.set_steam_id(Player.GetSteamId());
    PushMessage.set_unknown_3(0);

    if (!InitiatorClient->MessageStream->Send(&PushMessage))
    {
        WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectBreakInTarget to invader client %s.", InitiatorClient->GetName().c_str());
    }

    // Empty response, not sure what purpose this serves really other than saying message-recieved. Client
    // doesn't work without it though.
    DS2_Frpg2RequestMessage::RequestRejectVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectVisitResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_VisitorManager::GetName()
{
    return "Visitor";
}
