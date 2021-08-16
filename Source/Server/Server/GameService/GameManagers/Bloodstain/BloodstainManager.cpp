/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/Bloodstain/BloodstainManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

BloodstainManager::BloodstainManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

MessageHandleResult BloodstainManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestCreateBloodstain)
    {
        return Handle_RequestCreateBloodstain(Client, Message);
    }
    else if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestGetBloodstainList)
    {
        return Handle_RequestGetBloodstainList(Client, Message);
    }

    return MessageHandleResult::Unhandled;
}

MessageHandleResult BloodstainManager::Handle_RequestCreateBloodstain(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestCreateBloodstain* Request = (Frpg2RequestMessage::RequestCreateBloodstain*)Message.Protobuf.get();

    // TODO: Implement

    return MessageHandleResult::Handled;
}

MessageHandleResult BloodstainManager::Handle_RequestGetBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestGetBloodstainList* Request = (Frpg2RequestMessage::RequestGetBloodstainList*)Message.Protobuf.get();

    // TODO: Implement

    Frpg2RequestMessage::RequestGetBloodstainListResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestCreateBloodMessageResponse response.", Client->GetName().c_str());
        return MessageHandleResult::Error;
    }

    return MessageHandleResult::Handled;
}

std::string BloodstainManager::GetName()
{
    return "Bloodstain";
}
