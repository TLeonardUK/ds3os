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

PlayerDataManager::PlayerDataManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

bool PlayerDataManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateLoginPlayerCharacter)
    {
        return Handle_RequestUpdateLoginPlayerCharacter(Client, Message);
    }
    return false;
}

bool PlayerDataManager::Handle_RequestUpdateLoginPlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Log("[%s] RequestUpdateLoginPlayerCharacter from client.", Client->GetName().c_str());

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
        Warning("[%s] Disconnecting client as failed to send RequestUpdateLoginPlayerCharacterResponse response.", GetName().c_str());
        return true;
    }
    
    return false;
}

bool PlayerDataManager::Handle_RequestUpdatePlayerStatus(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Log("[%s] RequestUpdatePlayerStatus from client.", Client->GetName().c_str());

    Frpg2RequestMessage::RequestUpdatePlayerStatus* Request = (Frpg2RequestMessage::RequestUpdatePlayerStatus*)Message.Protobuf.get();

    // TODO: Do something with this data?

    // I don't think this response is even required, the normal server does not send it.
    Frpg2RequestMessage::RequestUpdatePlayerStatusResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdatePlayerStatusResponse response.", GetName().c_str());
        return true;
    }

    return false;
}

bool PlayerDataManager::Handle_RequestUpdatePlayerCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Log("[%s] RequestUpdatePlayerCharacter from client.", Client->GetName().c_str());

    Frpg2RequestMessage::RequestUpdatePlayerCharacter* Request = (Frpg2RequestMessage::RequestUpdatePlayerCharacter*)Message.Protobuf.get();

    // TODO: Do something with this data?

    // I don't think this response is even required, the normal server does not send it.
    Frpg2RequestMessage::RequestUpdatePlayerCharacterResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdatePlayerCharacterResponse response.", GetName().c_str());
        return true;
    }

    return false;
}

std::string PlayerDataManager::GetName()
{
    return "Player Data";
}
