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
#include "Server/GameService//Utils/DS2_GameIds.h"

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
    else if (Message.Header.IsType(DS2_Frpg2ReliableUdpMessageType::RequestGetLoginPlayerCharacter))
    {
        return Handle_RequestGetLoginPlayerCharacter(Client, Message);
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

    // If no character Id is specified we need to select one.
    uint32_t SelectedChracterId = Request->character_id();

    if (Request->character_id() == 0)
    {
        for (uint32_t i = 1; /* empty */; i++)
        {
            bool ValidId = true;

            for (size_t j = 0; j < Request->local_character_ids_size(); j++)
            {
                if (i == Request->local_character_ids(j))
                {
                    ValidId = false;
                    break;
                }
            }

            std::shared_ptr<Character> Char = Database.FindCharacter(State.GetPlayerId(), i);
            if (Char)
            {
                ValidId = false;
            }

            if (ValidId)
            {
                SelectedChracterId = i;
                break;
            }
        }
    }

    std::shared_ptr<Character> Char = Database.FindCharacter(State.GetPlayerId(), SelectedChracterId);
    if (!Char)
    {
        std::vector<uint8_t> Data;        
        if (!Database.CreateOrUpdateCharacter(State.GetPlayerId(), SelectedChracterId, Data))
        {
            WarningS(Client->GetName().c_str(), "Disconnecting client as failed to find or update character %i.", SelectedChracterId);
            return MessageHandleResult::Error;
        }

        Char = Database.FindCharacter(State.GetPlayerId(), SelectedChracterId);
        Ensure(Char);
    }

    State.SetCharacterId(Char->CharacterId);

    DS2_Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_character_id(Char->CharacterId);

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
    // TODO
    if (status.has_player_status() && status.player_status().played_areas_size() > 0)
    {
        State.GetPlayerStatus_Mutable().mutable_player_status()->clear_played_areas();
    }
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
        DS2_OnlineAreaId AreaId = static_cast<DS2_OnlineAreaId>(State.GetPlayerStatus().player_location().online_area_id());
        if (AreaId != State.GetCurrentArea() && AreaId != DS2_OnlineAreaId::None)
        {
            VerboseS(Client->GetName().c_str(), "User has entered '%s'", GetEnumString(AreaId).c_str());
            State.SetCurrentArea(AreaId);
        }
    }

    // Grab some matchmaking values.
    // TODO

    DS2_Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.");
        return MessageHandleResult::Error;
    }

    State.SetHasInitialState(true);


#if 0

    static DiffTracker Tracker;
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_2", State.GetPlayerStatus().player_status().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_4", State.GetPlayerStatus().player_status().unknown_4());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_5", State.GetPlayerStatus().player_status().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_6", State.GetPlayerStatus().player_status().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_7", State.GetPlayerStatus().player_status().unknown_7());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.character_id", State.GetPlayerStatus().player_status().character_id());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_11", State.GetPlayerStatus().player_status().unknown_11());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_13", State.GetPlayerStatus().player_status().unknown_13());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_14", State.GetPlayerStatus().player_status().unknown_14());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_16", State.GetPlayerStatus().player_status().unknown_16());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_17", State.GetPlayerStatus().player_status().unknown_17());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_18", State.GetPlayerStatus().player_status().unknown_18());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_20", State.GetPlayerStatus().player_status().unknown_20());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_status.unknown_21", State.GetPlayerStatus().player_status().unknown_21());
    
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.server_side_status.unknown_1", State.GetPlayerStatus().server_side_status().unknown_1());
    
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_location.unknown_2", State.GetPlayerStatus().player_location().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_location.unknown_3", State.GetPlayerStatus().player_location().unknown_3());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.player_location.unknown_5", State.GetPlayerStatus().player_location().unknown_5());

    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_1", State.GetPlayerStatus().item_using_info().unknown_1());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_2", State.GetPlayerStatus().item_using_info().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_3", State.GetPlayerStatus().item_using_info().unknown_3());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_4", State.GetPlayerStatus().item_using_info().unknown_4());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_5", State.GetPlayerStatus().item_using_info().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_6", State.GetPlayerStatus().item_using_info().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_7", State.GetPlayerStatus().item_using_info().unknown_7());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.item_using_info.unknown_8", State.GetPlayerStatus().item_using_info().unknown_8());

    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_1", State.GetPlayerStatus().stats_info().unknown_1());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_2", State.GetPlayerStatus().stats_info().unknown_2());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_3", State.GetPlayerStatus().stats_info().unknown_3());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_5", State.GetPlayerStatus().stats_info().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_10", State.GetPlayerStatus().stats_info().unknown_10());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_12", State.GetPlayerStatus().stats_info().unknown_12());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_13", State.GetPlayerStatus().stats_info().unknown_13());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_14", State.GetPlayerStatus().stats_info().unknown_14());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_15", State.GetPlayerStatus().stats_info().unknown_15());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_16", State.GetPlayerStatus().stats_info().unknown_16());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_18", State.GetPlayerStatus().stats_info().unknown_18());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.stats_info.unknown_20", State.GetPlayerStatus().stats_info().unknown_20());

    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_4", State.GetPlayerStatus().physical_status().unknown_4());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_5", State.GetPlayerStatus().physical_status().unknown_5());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_6", State.GetPlayerStatus().physical_status().unknown_6());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_7", State.GetPlayerStatus().physical_status().unknown_7());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_8", State.GetPlayerStatus().physical_status().unknown_8());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_9", State.GetPlayerStatus().physical_status().unknown_9());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_10", State.GetPlayerStatus().physical_status().unknown_10());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_11", State.GetPlayerStatus().physical_status().unknown_11());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_12", State.GetPlayerStatus().physical_status().unknown_12());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_13", State.GetPlayerStatus().physical_status().unknown_13());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_14", State.GetPlayerStatus().physical_status().unknown_14());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_15", State.GetPlayerStatus().physical_status().unknown_15());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_16", State.GetPlayerStatus().physical_status().unknown_16());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_17", State.GetPlayerStatus().physical_status().unknown_17());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_18", State.GetPlayerStatus().physical_status().unknown_18());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_19", State.GetPlayerStatus().physical_status().unknown_19());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_20", State.GetPlayerStatus().physical_status().unknown_20());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_21", State.GetPlayerStatus().physical_status().unknown_21());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_22", State.GetPlayerStatus().physical_status().unknown_22());
    Tracker.Field(State.GetCharacterName().c_str(), "PlayerStatus.physical_status.unknown_23", State.GetPlayerStatus().physical_status().unknown_23());

#endif

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

MessageHandleResult DS2_PlayerDataManager::Handle_RequestGetLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    ServerDatabase& Database = ServerInstance->GetDatabase();

    DS2_Frpg2RequestMessage::RequestGetLoginPlayerCharacter* Request = (DS2_Frpg2RequestMessage::RequestGetLoginPlayerCharacter*)Message.Protobuf.get();
    DS2_Frpg2RequestMessage::RequestGetLoginPlayerCharacterResponse Response;
    Response.set_player_id(Request->player_id());

    std::shared_ptr<GameClient> RequestedClient = ServerInstance->GetService<GameService>()->FindClientByPlayerId(Request->player_id());

    int CharacterId = 0;
    std::vector<uint8_t> CharacterData;

    if (RequestedClient != nullptr)
    {
        PlayerState& State = RequestedClient->GetPlayerState();

        std::shared_ptr<Character> Character = Database.FindCharacter(Request->player_id(), State.GetCharacterId());
        if (Character)
        {
            CharacterData = Character->Data;
            CharacterId = Character->CharacterId;
        }
    }

    Response.set_character_id(CharacterId);
    Response.set_character_data(CharacterData.data(), CharacterData.size());

    if (!Client->MessageStream->Send(&Response, &Message))
    {
        WarningS(Client->GetName().c_str(), "Disconnecting client as failed to send RequestGetLoginPlayerCharacterResponse response.");
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string DS2_PlayerDataManager::GetName()
{
    return "Player Data";
}
