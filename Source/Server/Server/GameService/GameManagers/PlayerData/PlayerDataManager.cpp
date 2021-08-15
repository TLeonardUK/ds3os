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

    return MessageHandleResult::Unhandled;
}

MessageHandleResult PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    // TODO: Do this in a better way.

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

    // Keep track of the players character name, useful for logging.
    if (Request->status().has_player_status() && Request->status().player_status().has_name())
    {
        PlayerState& State = Client->GetPlayerState();

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

    // TODO: Do something with this data?

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

    // TODO: Do something with this data?

    // I don't think this response is even required, the normal server does not send it.
    Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string PlayerDataManager::GetName()
{
    return "Player Data";
}
