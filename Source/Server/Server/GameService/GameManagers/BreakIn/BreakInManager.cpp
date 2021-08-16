/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BreakIn/BreakInManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

BreakInManager::BreakInManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult BreakInManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBreakInTargetList)
    {
        return Handle_RequestGetBreakInTargetList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestBreakInTarget)
    {
        return Handle_RequestBreakInTarget(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectBreakInTarget)
    {
        return Handle_RequestRejectBreakInTarget(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult BreakInManager::Handle_RequestGetBreakInTargetList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetBreakInTargetList* Request = (Frpg2RequestMessage::RequestGetBreakInTargetList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetBreakInTargetListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetBreakInTargetListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BreakInManager::Handle_RequestBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestBreakInTarget* Request = (Frpg2RequestMessage::RequestBreakInTarget*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestBreakInTargetResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult BreakInManager::Handle_RequestRejectBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRejectBreakInTarget* Request = (Frpg2RequestMessage::RequestRejectBreakInTarget*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRejectBreakInTargetResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectBreakInTargetResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BreakInManager::GetName()
{
    return "Break In";
}
