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
    Ensure(Request->unknown_1() == 1);
    Ensure(Request->unknown_2() == 1);

    Frpg2RequestMessage::RequestUpdateLoginPlayerCharacterResponse Response;
    Response.set_unknown_1(1);

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
        if (AreaId != State.CurrentArea)
        {
            Log("[%s] User has entered '%s'", Client->GetName().c_str(), GetEnumString(AreaId).c_str());
            State.CurrentArea = AreaId;
        }
    }

    // TODO: Do something with this data?
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

    Log("===== Player Character: %i", Request->unknown_1());

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
