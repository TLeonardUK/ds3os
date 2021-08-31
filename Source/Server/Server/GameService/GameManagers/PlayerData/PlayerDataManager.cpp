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
    // TODO: Implement
    // I'm not sure what the purpose of this request even is to be honest, it seems to occur when you switch characters and enter a game
    // but all the values are always the same? Maybe its just to tell the server our char has changed and to reset everything?

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter* Request = (Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter*)Message.Protobuf.get();
    //Ensure(Request->unknown_1() == 1);
    //Ensure(Request->unknown_2() == 1);

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Request->character_id());

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponseData* Data = Response.mutable_unknown_2();
    Data->set_unknown_1(0);
    Data->set_unknown_2(0);

    Data = Response.mutable_unknown_3();
    Data->set_unknown_1(0);
    Data->set_unknown_2(0);

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
        uint32_t LowerArea = Request->status().player_location().online_area_id_lower();
        uint32_t UpperArea = Request->status().player_location().online_area_id_upper();
        OnlineAreaId AreaId = static_cast<OnlineAreaId>(UpperArea);
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
            Log("[%s] User is now in visitor pool %i.", Client->GetName().c_str(), NewVisitorPool);
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
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_49)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_50)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_51)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_52)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_53)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_54)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_55)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_56)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_57)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_58)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_59)
    CHECK_PLAYER_STATUS_DIFF(equipment, unknown_60)
    // DEBUG DEBUG DEBUG

    // Merge the delta into the current state.
    State.PlayerStatus.MergeFrom(Request->status());

    // I don't think this response is even required, the normal server does not send it.
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
    Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement

    // I don't think this response is even required, the normal server does not send it.
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
    Frpg2RequestMessage::RequestGetPlayerCharacter* Request = (Frpg2RequestMessage::RequestGetPlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetPlayerCharacterResponse Response;
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
