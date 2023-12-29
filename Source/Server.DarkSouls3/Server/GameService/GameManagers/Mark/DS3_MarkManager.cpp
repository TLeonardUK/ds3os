/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Mark/DS3_MarkManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/DS3_Frpg2ReliableUdpMessage.h"
#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Shared/Core/Utils/Logging.h"

DS3_MarkManager::DS3_MarkManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult DS3_MarkManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestCreateMark))
    {
        return Handle_RequestCreateMark(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestRemoveMark))
    {
        return Handle_RequestRemoveMark(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestReentryMark))
    {
        return Handle_RequestReentryMark(Client, Message);
    }
    else if (Message.Header.IsType(DS3_Frpg2ReliableUdpMessageType::RequestGetMarkList))
    {
        return Handle_RequestGetMarkList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult DS3_MarkManager::Handle_RequestCreateMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestCreateMark* Request = (DS3_Frpg2RequestMessage::RequestCreateMark*)Message.Protobuf.get();

    // These functions should never be called, they are cut-content.
    Ensure(false);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_MarkManager::Handle_RequestRemoveMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestRemoveMark* Request = (DS3_Frpg2RequestMessage::RequestRemoveMark*)Message.Protobuf.get();

    // These functions should never be called, they are cut-content.
    Ensure(false);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_MarkManager::Handle_RequestReentryMark(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestReentryMark* Request = (DS3_Frpg2RequestMessage::RequestReentryMark*)Message.Protobuf.get();

    // These functions should never be called, they are cut-content.
    Ensure(false);

    return MessageHandleResult::Handled;
}

MessageHandleResult DS3_MarkManager::Handle_RequestGetMarkList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    DS3_Frpg2RequestMessage::RequestGetMarkList* Request = (DS3_Frpg2RequestMessage::RequestGetMarkList*)Message.Protobuf.get();

    // These functions should never be called, they are cut-content.
    Ensure(false);

    return MessageHandleResult::Handled;
}

std::string DS3_MarkManager::GetName()
{
    return "Mark";
}
