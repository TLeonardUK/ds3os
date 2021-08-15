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

    return MessageHandleResult::Unhandled;
}

MessageHandleResult LoggingManager::Handle_RequestNotifyProtoBufLog(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestNotifyProtoBufLog* Request = (Frpg2RequestMessage::RequestNotifyProtoBufLog*)Message.Protobuf.get();

    // TODO: Not sure what we want to do with this information really, its not really very useful for us.
    Log("[%s] Recieved protobuf log from client of type 0x%08x", Client->GetName().c_str(), Request->type());

    return MessageHandleResult::Handled;
}

std::string LoggingManager::GetName()
{
    return "Logging";
}
