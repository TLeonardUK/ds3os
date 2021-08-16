/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Ghosts/GhostManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

GhostManager::GhostManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult GhostManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetDeadingGhost)
    {
        return Handle_RequestGetDeadingGhost(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateGhostData)
    {
        return Handle_RequestCreateGhostData(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetGhostDataList)
    {
        return Handle_RequestGetGhostDataList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult GhostManager::Handle_RequestGetDeadingGhost(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetDeadingGhost* Request = (Frpg2RequestMessage::RequestGetDeadingGhost*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetDeadingGhostResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetDeadingGhostResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult GhostManager::Handle_RequestCreateGhostData(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateGhostData* Request = (Frpg2RequestMessage::RequestCreateGhostData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCreateGhostDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateGhostDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult GhostManager::Handle_RequestGetGhostDataList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetGhostDataList* Request = (Frpg2RequestMessage::RequestGetGhostDataList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetGhostDataListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetGhostDataListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string GhostManager::GetName()
{
    return "Ghosts";
}
