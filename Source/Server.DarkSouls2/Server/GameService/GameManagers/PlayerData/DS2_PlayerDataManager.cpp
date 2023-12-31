/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/PlayerData/DS2_PlayerDataManager.h"
#include "Server/GameService/DS2_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS2_Frpg2ReliableUdpMessage.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DiffTracker.h"

#include "Shared/Core/Network/NetConnection.h"

DS2_PlayerDataManager::DS2_PlayerDataManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS2_PlayerDataManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUpdateLoginPlayerCharacter))
    {
        return Handle_RequestUpdateLoginPlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUpdatePlayerStatus))
    {
        return Handle_RequestUpdatePlayerStatus(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestUpdatePlayerCharacter))
    {
        return Handle_RequestUpdatePlayerCharacter(Client, Message);
    }
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetPlayerCharacter))
    {
        return Handle_RequestGetPlayerCharacter(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS2_PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter* Request = (DS2_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacter*)Message.Protobuf.get();

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

    DS2_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Request->character_id());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }
    
    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS2_Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (DS2_Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    auto& State = Client->GetPlayerStateType<DS2_PlayerState>();

    // Merge the delta into the current state.
    std::string bytes = Request->status();

    DS2_Frpg2PlayerData::AllStatus status;
    if (!status.ParseFromArray(bytes.data(), (int)bytes.size()))
    {
        WarningS(Client->GetName().c_str(), "Failed to parse DS2_Frpg2PlayerData::AllStatus from RequestUpdatePlayerStatus.");

        // Don't take this as an error, it will resolve itself on next send.
        return MessageHandleResult::Handled;
    }

    // MergeFrom combines arrays, so we need to do some fuckyness here to handle this.
    /*
    if (status.has_player_status() && status.player_status().played_areas_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_played_areas();
    }
    if (status.has_player_status() && status.player_status().unknown_18_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_unknown_18();
    }
    if (status.has_player_status() && status.player_status().anticheat_data_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_anticheat_data();
    }
    */
    State.GetPlayerStatus_Mutable().MergeFrom(status);

    // Keep track of the players character id.
    /*
    if (State.GetPlayerStatus().player_status().has_character_id())
    {
        State.SetCharacterId(State.GetPlayerStatus().player_status().character_id());
    }
    */

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

    DS2_Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.");
        return MessageHandleResult::Error;
    }

    State.SetHasInitialState(true);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_PlayerDataManager::Handle_RequestUpdatePlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (DS2_Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    std::vector<uint8_t> Data;
    Data.assign(Request->character_data().data(), Request->character_data().data() + Request->character_data().size());

    if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), Request->character_id(), Data))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", Request->character_id());
        return MessageHandleResult::Error;
    }

    DS2_Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult DS2_PlayerDataManager::Handle_RequestGetPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();
    PlayerState& State = Client->GetPlayerState();

    DS2_Frpg2RequestMessage::RequestGetPlayerCharacter* Request = (DS2_Frpg2RequestMessage::RequestGetPlayerCharacter*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetPlayerCharacterResponse Response;

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

std::string DS2_PlayerDataManager::GetName()
{
    return "Player Data";
}
