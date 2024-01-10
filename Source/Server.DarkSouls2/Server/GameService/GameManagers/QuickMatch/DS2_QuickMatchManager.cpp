/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/QuickMatch/DS2_QuickMatchManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

DS2_QuickMatchManager::DS2_QuickMatchManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

void DS2_QuickMatchManager::OnLostPlayer(GameClient* Client)
{
    for (auto Iter = Matches.begin(); Iter != Matches.end(); /* empty */)
    {
        std::shared_ptr<Match> Match = *Iter;

        if (Match->HostPlayerId == Client->GetPlayerState().GetPlayerId())
        {
            LogS(Client->GetName().c_str(), "Unregistered quick match hosted by player %u, as player has disconnected.", Match->HostPlayerId);
            Iter = Matches.erase(Iter);
        }
        else
        {
            Iter++;
        }
    }
}

void DS2_QuickMatchManager::Poll()
{
}

MessageHandleResult DS2_QuickMatchManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestSearchQuickMatch))
    {
        return Handle_RequestSearchQuickMatch(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUnregisterQuickMatch))
    {
        return Handle_RequestUnregisterQuickMatch(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUpdateQuickMatch))
    {
        return Handle_RequestUpdateQuickMatch(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestJoinQuickMatch))
    {
        return Handle_RequestJoinQuickMatch(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRejectQuickMatch))
    {
        return Handle_RequestRejectQuickMatch(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestRegisterQuickMatch))
    {
        return Handle_RequestRegisterQuickMatch(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool DS2_QuickMatchManager::CanMatchWith(GameClient* Client, const DS2_Frpg2RequestMessage::RequestSearchQuickMatch& Request, const std::shared_ptr<Match>& Match)
{
    auto& Player = Client->GetPlayerStateType<DS2_PlayerState>();
    const RuntimeConfig& Config = ServerInstance->GetConfig();

    // Can't match with self.
    if (Client->GetPlayerState().GetPlayerId() == Match->HostPlayerId)
    {
        return false;
    }

    // Matches requested game mode.
    if (Match->GameMode != Request.mode())
    {
        return false;
    }

    // Matches requested map.
    if (Request.cell_id() != (uint32_t)Match->CellId ||
        Request.online_area_id() != (uint32_t)Match->AreaId)
    {
        return false;
    }

    // Check for matchmaking match.
    return Config.DS2_ArenaMatchingParameters.CheckMatch(Match->MatchingParams.soul_memory(), Request.matching_parameter().soul_memory(), Request.matching_parameter().name_engraved_ring() > 0);

#if 0

    // Can match with the hosts level.
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    const RuntimeConfigMatchingParameters* MatchingParams = &Config.UndeadMatchMatchingParameters;

    if (!MatchingParams->CheckMatch(
        Match->MatchingParams.soul_level(), Match->MatchingParams.weapon_level(),
        Request.matching_parameter().soul_level(), Request.matching_parameter().weapon_level(),
        Match->MatchingParams.password().size() > 0
    ))
    {
        return false;
    }

    // Check passwords match.
    if (Match->MatchingParams.password() != Request.matching_parameter().password())
    {
        return false;
    }
#endif

    return true;
}

std::shared_ptr<DS2_QuickMatchManager::Match> DS2_QuickMatchManager::GetMatchByHost(uint32_t HostPlayerId)
{
    for (std::shared_ptr<Match>& Iter : Matches)
    {
        if (Iter->HostPlayerId == HostPlayerId)
        {
            return Iter;
        }
    }
    return nullptr;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestSearchQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestSearchQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestSearchQuickMatch*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestSearchQuickMatchResponse Response;
    
    int ResultCount = 0;
    for (std::shared_ptr<Match>& Iter : Matches)
    {
        if (!CanMatchWith(Client, *Request, Iter))
        {
            continue;
        }

        DS2_Frpg2RequestMessage::QuickMatchData* Result = Response.add_matches();
        Result->set_player_id(Iter->HostPlayerId);
        Result->set_player_steam_id(Iter->HostPlayerSteamId);
        Result->set_cell_id((uint32_t)Iter->CellId);
        Result->mutable_matching_parameter()->CopyFrom(Iter->MatchingParams);
        Result->set_online_area_id((uint32_t)Iter->AreaId);
        Result->set_mode(Iter->GameMode);

        ResultCount++;
    }

    LogS(Client->GetName().c_str(), "RequestSearchQuickMatch: Found %i matches.", ResultCount);
    
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestCountRankingDataResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestRegisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestRegisterQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestRegisterQuickMatch*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestRegisterQuickMatchResponse Response;

    std::shared_ptr<Match> NewMatch = std::make_shared<Match>();
    NewMatch->HostPlayerId = Client->GetPlayerState().GetPlayerId();
    NewMatch->HostPlayerSteamId = Client->GetPlayerState().GetSteamId();
    NewMatch->GameMode = Request->mode();
    NewMatch->MatchingParams = Request->matching_parameter();
    NewMatch->CellId = Request->cell_id();
    NewMatch->AreaId = (DS2_OnlineAreaId)Request->online_area_id();
    NewMatch->HasStarted = false;

    LogS(Client->GetName().c_str(), "RequestRegisterQuickMatch: Hosting new match.");
    
    Matches.push_back(NewMatch);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRegisterQuickMatchResponse response.");
        return MessageHandleResult::Error;
    }

    if (ServerInstance->GetConfig().SendDiscordNotice_QuickMatch)
    {
        std::string ModeName = "";
        switch (NewMatch->GameMode)
        {
        case DS2_Frpg2RequestMessage::QuickMatchGameMode::QuickMatchGameMode_Blue:          ModeName = "Blue Sentinel";              break;
        case DS2_Frpg2RequestMessage::QuickMatchGameMode::QuickMatchGameMode_Brotherhood:   ModeName = "Brotherhood of Blood";    break;
        }

        ServerInstance->SendDiscordNotice(Client->shared_from_this(), DiscordNoticeType::UndeadMatch,
            StringFormat("Started a public '%s' undead match.", ModeName.c_str()),
            0,
            {
                { "Soul Memory", std::to_string(Client->GetPlayerState().GetSoulMemory()), true },
            }
        );
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestUnregisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestUnregisterQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestUnregisterQuickMatch*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestUnregisterQuickMatchResponse Response;

    for (auto Iter = Matches.begin(); Iter != Matches.end(); /* empty */)
    {
        std::shared_ptr<Match> Match = *Iter;
        if (          Match->HostPlayerId == Client->GetPlayerState().GetPlayerId() &&
                      Match->GameMode == Request->mode() &&
                      Match->CellId == Request->cell_id() &&
            (uint32_t)Match->AreaId == Request->online_area_id())
        {
            LogS(Client->GetName().c_str(), "RequestUnregisterQuickMatch: Unregistered quick match hosted by self.", Match->HostPlayerId);
            Iter = Matches.erase(Iter);
        }
        else
        {
            Iter++;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUnregisterQuickMatchResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestUpdateQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestUpdateQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestUpdateQuickMatch*)Message.Protobuf.get();

    // Not sure we really need to do anything with this. It just keeps the match alive?

    DS2_Frpg2RequestMessage::RequestUpdateQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateQuickMatchResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestJoinQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    DS2_PlayerState& Player = Client->GetPlayerStateType<DS2_PlayerState>();

    DS2_Frpg2RequestMessage::RequestJoinQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestJoinQuickMatch*)Message.Protobuf.get();

    bool bSuccess = false;

    std::shared_ptr<GameClient> HostClient = GameServiceInstance->FindClientByPlayerId(Request->player_id());
    if (HostClient)
    {
        std::shared_ptr<Match> ExistingMatch = GetMatchByHost(Request->player_id());    
        if (ExistingMatch)
        {        
            LogS(Client->GetName().c_str(), "RequestJoinQuickMatch: Attempting to join match hosted by %s", HostClient->GetName().c_str());

            DS2_Frpg2RequestMessage::PushRequestJoinQuickMatch PushMessage;
            PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestJoinQuickMatch);
            PushMessage.set_player_id(Player.GetPlayerId());
            PushMessage.set_player_steam_id(Player.GetSteamId());
            PushMessage.set_online_area_id((uint32_t)ExistingMatch->AreaId);
            PushMessage.set_cell_id((uint32_t)ExistingMatch->CellId);
            PushMessage.set_mode(ExistingMatch->GameMode);

            if (!HostClient->MessageStream->Send(&PushMessage))
            {
                WarningS(Client->GetName().c_str(), "Failed to send PushRequestJoinQuickMatch to host of quick match.");
                bSuccess = false;
            }
            else
            {
                bSuccess = true;
            }
        }
    }

    if (!bSuccess)
    {
        DS2_Frpg2RequestMessage::PushRequestRejectQuickMatch PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectQuickMatch);
        PushMessage.set_player_id(Request->player_id());
        PushMessage.set_player_steam_id(HostClient ? HostClient->GetPlayerState().GetSteamId() : "");
        PushMessage.set_online_area_id(Request->online_area_id());
        PushMessage.set_cell_id(Request->cell_id());
        PushMessage.set_mode(Request->mode());
        PushMessage.set_unknown_7(0);
     
        if (!Client->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectQuickMatch to player attempting to join quick match.");
            bSuccess = false;
        }

        return MessageHandleResult::Handled;
    }

    DS2_Frpg2RequestMessage::RequestJoinQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestJoinQuickMatchResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_QuickMatchManager::Handle_RequestRejectQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestRejectQuickMatch* Request = (DS2_Frpg2RequestMessage::RequestRejectQuickMatch*)Message.Protobuf.get();

    if (std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->player_id()))
    {
        LogS(Client->GetName().c_str(), "RequestRejectQuickMatch: Rejecting join request from %s", TargetClient->GetName().c_str());

        DS2_Frpg2RequestMessage::PushRequestRejectQuickMatch PushMessage;
        PushMessage.set_push_message_id(DS2_Frpg2RequestMessage::PushID_PushRequestRejectQuickMatch);
        PushMessage.set_player_id(Player.GetPlayerId());
        PushMessage.set_player_steam_id(Player.GetSteamId());
        PushMessage.set_online_area_id(Request->online_area_id());
        PushMessage.set_cell_id(Request->cell_id());
        PushMessage.set_mode(Request->mode());
        PushMessage.set_unknown_7(Request->unknown_5());

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            WarningS(Client->GetName().c_str(), "Failed to send PushRequestRejectQuickMatch to target of quick match join.");
        }
    }

    DS2_Frpg2RequestMessage::RequestRejectQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestRejectQuickMatchResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_QuickMatchManager::GetName()
{
    return "Quick Match";
}
