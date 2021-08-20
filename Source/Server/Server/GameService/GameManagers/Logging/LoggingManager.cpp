/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Logging/LoggingManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

LoggingManager::LoggingManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult LoggingManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyProtoBufLog)
    {
        return Handle_RequestNotifyProtoBufLog(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyKillEnemy)
    {
        return Handle_RequestNotifyProtoBufLog(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyDisconnectSession)
    {
        return Handle_RequestNotifyDisconnectSession(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyRegisterCharacter)
    {
        return Handle_RequestNotifyRegisterCharacter(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyDie)
    {
        return Handle_RequestNotifyDie(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyKillBoss)
    {
        return Handle_RequestNotifyKillBoss(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyJoinMultiplay)
    {
        return Handle_RequestNotifyJoinMultiplay(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifyLeaveMultiplay)
    {
        return Handle_RequestNotifyLeaveMultiplay(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestNotifySummonSignResult)
    {
        return Handle_RequestNotifySummonSignResult(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyProtoBufLog(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyProtoBufLog* Request = (Frpg2RequestMessage::RequestNotifyProtoBufLog*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyKillEnemy(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyKillEnemy* Request = (Frpg2RequestMessage::RequestNotifyKillEnemy*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyDisconnectSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyDisconnectSession* Request = (Frpg2RequestMessage::RequestNotifyDisconnectSession*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyRegisterCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyRegisterCharacter* Request = (Frpg2RequestMessage::RequestNotifyRegisterCharacter*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyDie(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyDie* Request = (Frpg2RequestMessage::RequestNotifyDie*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyKillBoss(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyKillBoss* Request = (Frpg2RequestMessage::RequestNotifyKillBoss*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyJoinMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyJoinMultiplay* Request = (Frpg2RequestMessage::RequestNotifyJoinMultiplay*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyLeaveMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyLeaveMultiplay* Request = (Frpg2RequestMessage::RequestNotifyLeaveMultiplay*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifySummonSignResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifySummonSignResult* Request = (Frpg2RequestMessage::RequestNotifySummonSignResult*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

std::string LoggingManager::GetName()
{
    return "Logging";
}
