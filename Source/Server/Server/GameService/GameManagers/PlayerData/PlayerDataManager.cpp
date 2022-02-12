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

    std::shared_ptr<Character> Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
    if (!Character)
    {
        std::vector<uint8_t> Data;        
        if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
            return MessageHandleResult::Error;
        }

        Character = Database.FindCharacter(State.GetPlayerId(), Request->character_id());
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
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    PlayerState& State = Client->GetPlayerState();

    // Merge the delta into the current state.
    std::string bytes = Request->status();

    Frpg2PlayerData::AllStatus status;
    if (!status.ParseFromArray(bytes.data(), (int)bytes.size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse Frpg2PlayerData::AllStatus from RequestUpdatePlayerStatus.");

        // Don't take this as an error, it will resolve itself on next send.
        return MessageHandleResult::Handled;
    }

    State.GetPlayerStatus_Mutable().MergeFrom(status);
    State.Mutated();

    // Keep track of the players character id.
    if (State.GetPlayerStatus().player_status().has_character_id())
    {
        State.SetCharacterId(State.GetPlayerStatus().player_status().character_id());
    }

    // Keep track of the players character name, useful for logging.
    if (State.GetPlayerStatus().player_status().has_name())
    {
        std::string NewCharacterName = State.GetPlayerStatus().player_status().name();
        if (State.GetCharacterName() != NewCharacterName)
        {
            State.SetCharacterName(NewCharacterName);

            std::string NewConnectionName = StringFormat("%i:%s", State.GetPlayerId(), State.GetCharacterName().c_str());

            LogS(Client->GetName().c_str(), "Renaming connection to '%s'.", NewConnectionName.c_str());

            // Rename connection after this point as it easier to keep track of than ip:port pairs.
            Client->Connection->Rename(NewConnectionName);
        }
    }

    // Print a log if user has changed online location.
    if (State.GetPlayerStatus().has_player_location())
    {
        OnlineAreaId AreaId = static_cast<OnlineAreaId>(State.GetPlayerStatus().player_location().online_area_id());
        if (AreaId != State.GetCurrentArea() && AreaId != OnlineAreaId::None)
        {
            VerboseS(Client->GetName().c_str(), "User has entered '%s'", GetEnumString(AreaId).c_str());
            State.SetCurrentArea(AreaId);
        }
    }

    // Grab some matchmaking values.
    if (State.GetPlayerStatus().has_player_status())
    {
        // Grab invadability state.
        if (State.GetPlayerStatus().player_status().has_is_invadable())
        {
            bool NewState = State.GetPlayerStatus().player_status().is_invadable();
            if (NewState != State.GetIsInvadable())
            {
                VerboseS(Client->GetName().c_str(), "User is now %s", NewState ? "invadable" : "no longer invadable");
                State.SetIsInvadable(NewState);
            }
        }

        // Grab soul level / weapon level.
        if (State.GetPlayerStatus().player_status().has_soul_level())
        {
            State.SetSoulLevel(State.GetPlayerStatus().player_status().soul_level());
        }
        if (State.GetPlayerStatus().player_status().has_max_weapon_level())
        {
            State.SetMaxWeaponLevel(State.GetPlayerStatus().player_status().max_weapon_level());
        }

        // Grab whatever visitor pool they should be in.
        Frpg2RequestMessage::VisitorPool NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_None;
        if (State.GetPlayerStatus().player_status().has_can_summon_for_way_of_blue() && State.GetPlayerStatus().player_status().can_summon_for_way_of_blue())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Way_of_Blue;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_watchdog_of_farron() && State.GetPlayerStatus().player_status().can_summon_for_watchdog_of_farron())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Watchdog_of_Farron;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_aldritch_faithful() && State.GetPlayerStatus().player_status().can_summon_for_aldritch_faithful())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Aldrich_Faithful;
        }
        if (State.GetPlayerStatus().player_status().has_can_summon_for_spear_of_church() && State.GetPlayerStatus().player_status().can_summon_for_spear_of_church())
        {
            NewVisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_Spear_of_the_Church;
        }

        if (NewVisitorPool != State.GetVisitorPool())
        {
            State.SetVisitorPool(NewVisitorPool);
        }
    }

    Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.");
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

    if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
        return MessageHandleResult::Error;
    }

    Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.");
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
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestGetLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetLoginPlayerCharacter* Request = (Frpg2RequestMessage::RequestGetLoginPlayerCharacter*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    Frpg2RequestMessage::RequestGetLoginPlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult PlayerDataManager::Handle_RequestGetPlayerCharacterList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetPlayerCharacterList* Request = (Frpg2RequestMessage::RequestGetPlayerCharacterList*)Message.Protobuf.get();

    // TODO: Implement
    Ensure(false); // Never seen this in use.

    Frpg2RequestMessage::RequestGetPlayerCharacterListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetPlayerCharacterListResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string PlayerDataManager::GetName()
{
    return "Player Data";
}
