/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/QuickMatch/QuickMatchManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

QuickMatchManager::QuickMatchManager(Server* InServerInstance, GameService* InGameServiceInstance)
    : ServerInstance(InServerInstance)
    , GameServiceInstance(InGameServiceInstance)
{
}

void QuickMatchManager::OnLostPlayer(GameClient* Client)
{
    for (auto Iter = Matches.begin(); Iter != Matches.end(); /* empty */)
    {
        std::shared_ptr<Match> Match = *Iter;

        if (Match->HostPlayerId == Client->GetPlayerState().PlayerId)
        {
            Log("[%s] Cleaned up undead match due to disconnecting client.", Client->GetName().c_str());
            Iter = Matches.erase(Iter);
        }
        else
        {
            Iter++;
        }
    }
}

void QuickMatchManager::Poll()
{
}

MessageHandleResult QuickMatchManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSearchQuickMatch)
    {
        return Handle_RequestSearchQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUnregisterQuickMatch)
    {
        return Handle_RequestUnregisterQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateQuickMatch)
    {
        return Handle_RequestUpdateQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestJoinQuickMatch)
    {
        return Handle_RequestJoinQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestAcceptQuickMatch)
    {
        return Handle_RequestAcceptQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectQuickMatch)
    {
        return Handle_RequestRejectQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRegisterQuickMatch)
    {
        return Handle_RequestRegisterQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendQuickMatchStart)
    {
        return Handle_RequestSendQuickMatchStart(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendQuickMatchResult)
    {
        return Handle_RequestSendQuickMatchResult(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

bool QuickMatchManager::CanMatchWith(GameClient* Client, const Frpg2RequestMessage::RequestSearchQuickMatch& Request, const std::shared_ptr<Match>& Match)
{
    // Matches requested game mode.
    if (Match->GameMode != Request.mode())
    {
        return false;
    }

    // Matches requested map.
    bool ValidMap = false;
    for (int i = 0; i < Request.map_id_list_size(); i++)
    {
        if (Request.map_id_list(i).map_id() == (uint32_t)Match->MapId &&
            Request.map_id_list(i).online_area_id() == (uint32_t)Match->AreaId)
        {
            ValidMap = true;
            break;
        }
    }
    if (!ValidMap)
    {
        return false;
    }

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

    return true;
}

std::shared_ptr<QuickMatchManager::Match> QuickMatchManager::GetMatchByHost(uint32_t HostPlayerId)
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

MessageHandleResult QuickMatchManager::Handle_RequestSearchQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestSearchQuickMatch* Request = (Frpg2RequestMessage::RequestSearchQuickMatch*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestSearchQuickMatchResponse Response;

    int ResultCount = 0;
    for (std::shared_ptr<Match>& Iter : Matches)
    {
        if (!CanMatchWith(Client, *Request, Iter))
        {
            continue;
        }

        Frpg2RequestMessage::QuickMatchSearchResult* Result = Response.add_matches();
        Result->mutable_data()->set_host_player_id(Iter->HostPlayerId);
        Result->mutable_data()->set_host_player_steam_id(Iter->HostPlayerSteamId);
        Result->mutable_data()->set_online_area_id((uint32_t)Iter->AreaId);
        Result->set_unknown_3(0);
        Result->set_unknown_4(0);

        ResultCount++;
    }

    // If there are no results server seems to send an empty match result, client fails without it.
    if (ResultCount == 0)
    {
        Frpg2RequestMessage::QuickMatchSearchResult* Result = Response.add_matches();
        Result->set_unknown_3(0);
        Result->set_unknown_4(0);
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCountRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestRegisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRegisterQuickMatch* Request = (Frpg2RequestMessage::RequestRegisterQuickMatch*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestRegisterQuickMatchResponse Response;

    std::shared_ptr<Match> NewMatch = std::make_shared<Match>();
    NewMatch->HostPlayerId = Client->GetPlayerState().PlayerId;
    NewMatch->HostPlayerSteamId = Client->GetPlayerState().SteamId;
    NewMatch->GameMode = Request->mode();
    NewMatch->MatchingParams = Request->matching_parameter();
    NewMatch->MapId = Request->map_id();
    NewMatch->AreaId = (OnlineAreaId)Request->online_area_id();
    NewMatch->HasStarted = false;

    Matches.push_back(NewMatch);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRegisterQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestUnregisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUnregisterQuickMatch* Request = (Frpg2RequestMessage::RequestUnregisterQuickMatch*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestUnregisterQuickMatchResponse Response;

    for (auto Iter = Matches.begin(); Iter != Matches.end(); /* empty */)
    {
        std::shared_ptr<Match> Match = *Iter;
        if (          Match->GameMode == Request->mode() &&
                      Match->MapId == Request->map_id() &&
            (uint32_t)Match->AreaId == Request->online_area_id())
        {
            Iter = Matches.erase(Iter);
        }
        else
        {
            Iter++;
        }
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUnregisterQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestUpdateQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdateQuickMatch* Request = (Frpg2RequestMessage::RequestUpdateQuickMatch*)Message.Protobuf.get();

    // Not sure we really need to do anything with this. It just keeps the match alive?

    Frpg2RequestMessage::RequestUpdateQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdateQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestJoinQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestJoinQuickMatch* Request = (Frpg2RequestMessage::RequestJoinQuickMatch*)Message.Protobuf.get();

    bool bSuccess = false;

    std::shared_ptr<GameClient> HostClient = GameServiceInstance->FindClientByPlayerId(Request->host_player_id());
    if (HostClient)
    {
        std::shared_ptr<Match> ExistingMatch = GetMatchByHost(Request->host_player_id());    
        if (ExistingMatch)
        {        
            Frpg2RequestMessage::PushRequestJoinQuickMatch PushMessage;
            PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestJoinQuickMatch);
            PushMessage.mutable_message()->set_join_player_id(Player.PlayerId);
            PushMessage.mutable_message()->set_join_player_steam_id(Player.SteamId);
            PushMessage.mutable_message()->set_unknown_3(1);                 // TODO: Figure out
            PushMessage.mutable_message()->set_online_area_id((uint32_t)ExistingMatch->AreaId);
            PushMessage.mutable_message()->set_unknown_5(0);                 // TODO: Figure out
            PushMessage.mutable_message()->set_unknown_6("");                // TODO: Figure out

            if (!HostClient->MessageStream->Send(&PushMessage))
            {
                Warning("[%s] Failed to send PushRequestJoinQuickMatch to host of quick match.", Client->GetName().c_str());
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
        Frpg2RequestMessage::PushRequestRejectQuickMatch PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectQuickMatch);
        PushMessage.mutable_message()->set_host_player_id(Request->host_player_id());
     
        if (HostClient)
        {
            PushMessage.mutable_message()->set_host_player_steam_id(HostClient->GetPlayerState().SteamId);
        }

        if (!Client->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestRejectQuickMatch to player attempting to join quick match.", Client->GetName().c_str());
            bSuccess = false;
        }

        return MessageHandleResult::Error;
    }

    Frpg2RequestMessage::RequestJoinQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestJoinQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestAcceptQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestAcceptQuickMatch* Request = (Frpg2RequestMessage::RequestAcceptQuickMatch*)Message.Protobuf.get();

    if (std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->join_player_id()))
    {    
        Frpg2RequestMessage::PushRequestAcceptQuickMatch PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestAcceptQuickMatch);
        PushMessage.mutable_message()->set_host_player_id(Player.PlayerId);
        PushMessage.mutable_message()->set_host_player_steam_id(Player.SteamId);
        PushMessage.mutable_message()->set_metadata(Request->data().data(), Request->data().size());

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestAcceptQuickMatch to target of quick match join.", Client->GetName().c_str());
        }
    }

    Frpg2RequestMessage::RequestAcceptQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestAcceptQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestRejectQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestRejectQuickMatch* Request = (Frpg2RequestMessage::RequestRejectQuickMatch*)Message.Protobuf.get();

    if (std::shared_ptr<GameClient> TargetClient = GameServiceInstance->FindClientByPlayerId(Request->join_player_id()))
    {
        Frpg2RequestMessage::PushRequestRejectQuickMatch PushMessage;
        PushMessage.set_push_message_id(Frpg2RequestMessage::PushID_PushRequestRejectQuickMatch);
        PushMessage.mutable_message()->set_host_player_id(Player.PlayerId);
        PushMessage.mutable_message()->set_host_player_steam_id(Player.SteamId);

        if (!TargetClient->MessageStream->Send(&PushMessage))
        {
            Warning("[%s] Failed to send PushRequestAcceptQuickMatch to target of quick match join.", Client->GetName().c_str());
        }
    }

    Frpg2RequestMessage::RequestRejectQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestSendQuickMatchStart(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    PlayerState& Player = Client->GetPlayerState();

    Frpg2RequestMessage::RequestSendQuickMatchStart* Request = (Frpg2RequestMessage::RequestSendQuickMatchStart*)Message.Protobuf.get();

    for (auto Iter = Matches.begin(); Iter != Matches.end(); Iter++)
    {
        std::shared_ptr<Match> Match = *Iter;
        if (Match->HostPlayerId == Player.PlayerId)
        {
            Matches.erase(Iter);
            break;
        }
    }

    Frpg2RequestMessage::RequestSendQuickMatchStartResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestSendQuickMatchStartResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestSendQuickMatchResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    const RuntimeConfig& Config = ServerInstance->GetConfig();
    PlayerState& State = Client->GetPlayerState();

    Frpg2RequestMessage::RequestSendQuickMatchResult* Request = (Frpg2RequestMessage::RequestSendQuickMatchResult*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestSendQuickMatchResultResponse Response;
    Response.set_unknown_1(0); // TODO: Figure out.

    // Grab the players character to get their current rank data.
    std::shared_ptr<Character> Character = Database.FindCharacter(State.PlayerId, State.CharacterId);
    if (!Character)
    {
        Warning("[%s] Disconnecting client as failed to find current character during QuickMatchResult.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    // Increase and return rank.
    bool IsDuel = (Request->mode() == Frpg2RequestMessage::QuickMatchGameMode::Duel);

    uint32_t OriginalRank = IsDuel ? Character->QuickMatchDuelRank : Character->QuickMatchBrawlRank;
    uint32_t OriginalXP = IsDuel ? Character->QuickMatchDuelXp : Character->QuickMatchBrawlXp;

    uint32_t& Rank = IsDuel ? Character->QuickMatchDuelRank : Character->QuickMatchBrawlRank;
    uint32_t& XP = IsDuel ? Character->QuickMatchDuelXp : Character->QuickMatchBrawlXp;
    
    switch (Request->result())
    {
        case Frpg2RequestMessage::QuickMatchResult::QuickMatchResult_Win:
        {
            XP += Config.QuickMatchWinXp;
            break;
        }
        case Frpg2RequestMessage::QuickMatchResult::QuickMatchResult_Draw:
        {
            XP += Config.QuickMatchDrawXp;
            break;
        }
        case Frpg2RequestMessage::QuickMatchResult::QuickMatchResult_Lose:
        {
            XP += Config.QuickMatchLoseXp;
            break;
        }
    }

    while (Rank < Config.QuickMatchRankXp.size() - 1)
    {
        uint32_t NextRankXP = Config.QuickMatchRankXp[Rank + 1];
        if (XP > NextRankXP)
        {
            Rank++;
            XP -= NextRankXP;
        }
        else
        {
            break;
        }
    }

    Response.mutable_new_local_rank()->set_rank(Rank);
    Response.mutable_new_local_rank()->set_xp(XP);

    Log("[%s] Player finished undead match, ranked up to: rank=%i xp=%i (from rank=%i xp=%i)", Client->GetName().c_str(), Rank, XP, OriginalRank, OriginalXP);

    // Update character state.
    if (!Database.UpdateCharacterQuickMatchRank(State.PlayerId, State.CharacterId, Character->QuickMatchDuelRank, Character->QuickMatchDuelXp, Character->QuickMatchBrawlRank, Character->QuickMatchBrawlXp))
    {
        Warning("[%s] Disconnecting client as failed to update their quick match result.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestSendQuickMatchResultResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string QuickMatchManager::GetName()
{
    return "Quick Match";
}
