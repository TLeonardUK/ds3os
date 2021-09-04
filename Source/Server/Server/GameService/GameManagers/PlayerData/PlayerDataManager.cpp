/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/PlayerData/PlayerDataManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

#include "Core/Network/NetConnection.h"

PlayerDataManager::PlayerDataManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult PlayerDataManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateLoginPlayerCharacter)
    {
        return Handle_RequestUpdateLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdatePlayerStatus)
    {
        return Handle_RequestUpdatePlayerStatus(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdatePlayerCharacter)
    {
        return Handle_RequestUpdatePlayerCharacter(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetPlayerCharacter)
    {
        return Handle_RequestGetPlayerCharacter(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetLoginPlayerCharacter)
    {
        return Handle_RequestGetLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetPlayerCharacterList)
    {
        return Handle_RequestGetPlayerCharacterList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter* Request = (Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter*)Message.Protobuf.get();

    std::shared_ptr<Character> Character = Database.FindCharacter(State.PlayerId, Request->character_id());
    if (!Character)
    {
        std::vector<uint8_t> Data;        
        if (!Database.CreateOrUpdateCharacter(State.PlayerId, Request->character_id(), Data))
        {
            Warning("[%s] Disconnecting client as failed to find or update character %i.", Client->GetName().c_str(), Request->character_id());
            return MessageHandleResult::Error;
        }

        Character = Database.FindCharacter(State.PlayerId, Request->character_id());
        Ensure(Character);
    }

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Request->character_id());

    Frpg2RequestMessage::QuickMatchRank* Rank = Response.mutable_quickmatch_brawl_rank();
    Rank->set_rank(Character->QuickMatchBrawlRank);
    Rank->set_xp(Character->QuickMatchBrawlXp);

    Rank = Response.mutable_quickmatch_dual_rank();
    Rank->set_rank(Character->QuickMatchDuelRank);
    Rank->set_xp(Character->QuickMatchDuelXp);

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    PlayerState& State = Client->GetPlayerState();

    // Keep track of the players character id.
    if (Request->status().has_player_status() && Request->status().player_status().has_character_id())
    {
        State.CharacterId = Request->status().player_status().character_id();
    }

    // Keep track of the players character name, useful for logging.
    if (Request->status().has_player_status() && Request->status().player_status().has_name())
    {
        std::string NewCharacterName = Request->status().player_status().name();
        if (State.CharacterName != NewCharacterName)
        {
            State.CharacterName = NewCharacterName;

            std::string NewConnectionName = StringFormat("%i:%s", State.PlayerId, State.CharacterName.c_str());

            Log("[%s] Renaming connection to '%s'.", Client->GetName().c_str(), NewConnectionName.c_str());

            // Rename connection after this point as it easier to keep track of than ip:port pairs.
            Client->Connection->Rename(NewConnectionName);
        }
    }

    // Print a log if user has changed online location.
    if (Request->status().has_player_location())
    {
        OnlineAreaId AreaId = static_cast<OnlineAreaId>(Request->status().player_location().online_area_id());
        if (AreaId != State.CurrentArea && AreaId != OnlineAreaId::None)
        {
            Log("[%s] User has entered '%s'", Client->GetName().c_str(), GetEnumString(AreaId).c_str());
            State.CurrentArea = AreaId;
        }
    }

    // Grab some matchmaking values.
    if (Request->status().has_player_status())
    {
        // Grab invadability state.
        if (Request->status().player_status().has_is_invadable())
        {
            bool NewState = Request->status().player_status().is_invadable();
            if (NewState != State.IsInvadable)
            {
                Log("[%s] User is now %s", Client->GetName().c_str(), NewState ? "invadable" : "no longer invadable");
                State.IsInvadable = NewState;
            }
        }

        // Grab soul level / weapon level.
        if (Request->status().player_status().has_soul_level())
        {
            State.SoulLevel = Request->status().player_status().soul_level();
        }
        if (Request->status().player_status().has_max_weapon_level())
        {
            State.MaxWeaponLevel = Request->status().player_status().max_weapon_level();
        }

        // Grab whatever visitor pool they should be in.
        Frpg2RequestMessage::VisitorPool NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_None;
        if (Request->status().player_status().has_can_summon_for_way_of_blue() && Request->status().player_status().can_summon_for_way_of_blue())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Way_of_Blue;
        }
        if (Request->status().player_status().has_can_summon_for_watchdog_of_farron() && Request->status().player_status().can_summon_for_watchdog_of_farron())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Watchdog_of_Farron;
        }
        if (Request->status().player_status().has_can_summon_for_aldritch_faithful() && Request->status().player_status().can_summon_for_aldritch_faithful())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Aldrich_Faithful;
        }
        if (Request->status().player_status().has_can_summon_for_spear_of_church() && Request->status().player_status().can_summon_for_spear_of_church())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Spear_of_the_Church;
        }

        if (NewVisitorPool != State.VisitorPool)
        {
            State.VisitorPool = NewVisitorPool;
        }
    }

    // DEBUG DEBUG DEBUG
    #define CHECK_PLAYER_STATUS_DIFF(GroupName, FieldName) if (State.PlayerStatus.has_##GroupName() && State.PlayerStatus.GroupName().has_##FieldName() && Request->status().has_##GroupName() && Request->status().GroupName().has_##FieldName() && State.PlayerStatus.GroupName().FieldName() != Request->status().GroupName().FieldName()) \
                                                    Warning("[%s] Changed %s.%s to %i.", Client->GetName().c_str(), #GroupName, #FieldName, Request->status().GroupName().FieldName());
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_1)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_2)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_6)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_9)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_13)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_14)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_32)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_33)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_63)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_75)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_76)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_78)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_79)
    CHECK_PLAYER_STATUS_DIFF(player_status, unknown_80)
    CHECK_PLAYER_STATUS_DIFF(play_data, unknown_4)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_59)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_60)
    // DEBUG DEBUG DEBUG

    // Merge the delta into the current state.
    State.PlayerStatus.MergeFrom(Request->status());

    Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestUpdatePlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->character_data().data(), Request->character_data().data() + Request->character_data().size());

    if (!Database.CreateOrUpdateCharacter(State.PlayerId, Request->character_id(), Data))
    {
        Warning("[%s] Disconnecting client as failed to find or update character %i.", Client->GetName().c_str(), Request->character_id());
        return MessageHandleResult::Error;
    }

    Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestGetPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    Frpg2RequestMessage::RequestGetPlayerCharacter* Request = (Frpg2RequestMessage::RequestGetPlayerCharacter*)Message.Protobuf.get();
    Frpg2RequestMessage::RequestGetPlayerCharacterResponse Response;

    std::vector<uint8_t> CharacterData;
    std::shared_ptr<Character> Character = Database.FindCharacter(Request->player_id(), Request->character_id());
    if (Character)
    {
        CharacterData = Character->Data;
    }

    Response.set_player_id(Request->player_id());
    Response.set_character_id(Request->character_id());
    Response.set_character_data(CharacterData.data(), CharacterData.size());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetPlayerCharacterResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestGetLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetLoginPlayerCharacter* Request = (Frpg2RequestMessage::RequestGetLoginPlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetLoginPlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetLoginPlayerCharacterResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestGetPlayerCharacterList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetPlayerCharacterList* Request = (Frpg2RequestMessage::RequestGetPlayerCharacterList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetPlayerCharacterListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetPlayerCharacterListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string PlayerDataManager::GetName()
{
    return "Player Data";
}
