/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Visitor/VisitorManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

VisitorManager::VisitorManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult VisitorManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetVisitorList)
    {
        return Handle_RequestGetVisitorList(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestVisit)
    {
        return Handle_RequestVisit(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectVisit)
    {
        return Handle_RequestRejectVisit(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult VisitorManager::Handle_RequestGetVisitorList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetVisitorList* Request = (Frpg2RequestMessage::RequestGetVisitorList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetVisitorListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetVisitorListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult VisitorManager::Handle_RequestVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestVisit* Request = (Frpg2RequestMessage::RequestVisit*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestVisitResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult VisitorManager::Handle_RequestRejectVisit(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRejectVisit* Request = (Frpg2RequestMessage::RequestRejectVisit*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRejectVisitResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectVisitResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string VisitorManager::GetName()
{
    return "Visitor";
}
