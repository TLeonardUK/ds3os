/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/BloodMessage/BloodMessageManager.h"
#include "Server/GameService/GameClient.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Config/RuntimeConfig.h"
#include "Server/Server.h"

#include "Core/Utils/Logging.h"

BloodMessageManager::BloodMessageManager(Server* InServerInstance)
    : ServerInstance(InServerInstance)
{
}

bool BloodMessageManager::OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    if (Message.Header.msg_type == Frpg2ReliableUdpMessageType::RequestReentryBloodMessage)
    {
        return Handle_RequestReentryBloodMessage(Client, Message);
    }
    return false;
}

bool BloodMessageManager::Handle_RequestReentryBloodMessage(GameClient* Client, const Frpg2ReliableUdpMessage& Message)
{
    Frpg2RequestMessage::RequestReentryBloodMessage* Request = (Frpg2RequestMessage::RequestReentryBloodMessage*)Message.Protobuf.get();
    Ensure(Request->unknown_2() == 1);

    // TODO: Actually do something!

    Frpg2RequestMessage::RequestReentryBloodMessageResponse Response;
    if (!Client->MessageStream->Send(&Response, &Message))
    {
        Warning("[%s] Disconnecting client as failed to send RequestReentryBloodMessageResponse response.", GetName().c_str());
        return true;
    }
    
    return false;
}

std::string BloodMessageManager::GetName()
{
    return "Blood Message";
}
