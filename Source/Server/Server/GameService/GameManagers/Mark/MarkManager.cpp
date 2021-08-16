/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Mark/MarkManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

MarkManager::MarkManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult MarkManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateMark)
    {
        return Handle_RequestCreateMark(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRemoveMark)
    {
        return Handle_RequestRemoveMark(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestReentryMark)
    {
        return Handle_RequestReentryMark(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetMarkList)
    {
        return Handle_RequestGetMarkList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult MarkManager::Handle_RequestCreateMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateMark* Request = (Frpg2RequestMessage::RequestCreateMark*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCreateMarkResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateMarkResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MarkManager::Handle_RequestRemoveMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRemoveMark* Request = (Frpg2RequestMessage::RequestRemoveMark*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRemoveMarkResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRemoveMarkResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MarkManager::Handle_RequestReentryMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestReentryMark* Request = (Frpg2RequestMessage::RequestReentryMark*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestReentryMarkResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestReentryMarkResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult MarkManager::Handle_RequestGetMarkList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetMarkList* Request = (Frpg2RequestMessage::RequestGetMarkList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetMarkListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestGetMarkListResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string MarkManager::GetName()
{
    return "Mark";
}
