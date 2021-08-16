/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/QuickMatch/QuickMatchManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

QuickMatchManager::QuickMatchManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult QuickMatchManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSearchQuickMatch)
    {
        return Handle_RequestSearchQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUnregisterQuickMatch)
    {
        return Handle_RequestUnregisterQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestUpdateQuickMatch)
    {
        return Handle_RequestUpdateQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestJoinQuickMatch)
    {
        return Handle_RequestJoinQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestAcceptQuickMatch)
    {
        return Handle_RequestAcceptQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRejectQuickMatch)
    {
        return Handle_RequestRejectQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestRegisterQuickMatch)
    {
        return Handle_RequestRegisterQuickMatch(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendQuickMatchStart)
    {
        return Handle_RequestSendQuickMatchStart(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestSendQuickMatchResult)
    {
        return Handle_RequestSendQuickMatchResult(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult QuickMatchManager::Handle_RequestSearchQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCountRankingData* Request = (Frpg2RequestMessage::RequestCountRankingData*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestCountRankingDataResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCountRankingDataResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestUnregisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUnregisterQuickMatch* Request = (Frpg2RequestMessage::RequestUnregisterQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestUnregisterQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUnregisterQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestUpdateQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestUpdateQuickMatch* Request = (Frpg2RequestMessage::RequestUpdateQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestUpdateQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestUpdateQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestJoinQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestJoinQuickMatch* Request = (Frpg2RequestMessage::RequestJoinQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestJoinQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestJoinQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestAcceptQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestAcceptQuickMatch* Request = (Frpg2RequestMessage::RequestAcceptQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestAcceptQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestAcceptQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestRejectQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRejectQuickMatch* Request = (Frpg2RequestMessage::RequestRejectQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestRejectQuickMatchResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestRejectQuickMatchResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestRegisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestRegisterQuickMatch* Request = (Frpg2RequestMessage::RequestRegisterQuickMatch*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestSendQuickMatchStart(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestSendQuickMatchStart* Request = (Frpg2RequestMessage::RequestSendQuickMatchStart*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult QuickMatchManager::Handle_RequestSendQuickMatchResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestSendQuickMatchResult* Request = (Frpg2RequestMessage::RequestSendQuickMatchResult*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestSendQuickMatchResultResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestSendQuickMatchResultResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string QuickMatchManager::GetName()
{
    return "Quick Match";
}
