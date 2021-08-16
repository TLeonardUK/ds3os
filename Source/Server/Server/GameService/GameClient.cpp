/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameManager.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"

#include "Server/Server.h"

#include "Platform/Platform.h"

#include "Core/Utils/Logging.h"
#include "Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

GameClient::GameClient(GameService* OwningService, std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& CwcKey, uint64_t InAuthToken)
    : Service(OwningService)
    , Connection(InConnection)
    , AuthToken(InAuthToken)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2ReliableUdpMessageStream>(InConnection, CwcKey, AuthToken);
}

bool GameClient::Poll()
{
    // Has this client timed out?
    double TimeSinceLastMessage = GetSeconds() - LastMessageRecievedTime;
    if (TimeSinceLastMessage >= BuildConfig::CLIENT_TIMEOUT)
    {
        Warning("[%s] Client timed out.", GetName().c_str());
        return true;
    }

    // Client disconnected.
    if (Connection->Pump())
    {
        Warning("[%s] Disconnecting client as connection was in an error state.", GetName().c_str());
        return true;
    }
    if (!Connection->IsConnected())
    {
        Warning("[%s] Client disconnected.", GetName().c_str());
        return true;
    }

    // Pump the message stream and handle any messages that come in.
    if (MessageStream->Pump())
    {
        Warning("[%s] Disconnecting client as message stream closed.", GetName().c_str());
        return true;
    }

    // Process all packets.
    Frpg2ReliableUdpMessage Message;
    while (MessageStream->Recieve(&Message))
    {
        if (HandleMessage(Message))
        {
            if (BuildConfig::DISCONNECT_ON_UNHANDLED_MESSAGE)
            {
                Warning("[%s] Disconnecting client as failed to handle message.", GetName().c_str());
                return true;
            }
            else
            {
                Warning("[%s] Failed to handle message, ignoring and hoping nothing breaks ...", GetName().c_str());
            }
        }

        // TODO: Find a better way to do this that doesn't break our abstraction.
        MessageStream->HandledPacket(Message.AckSequenceIndex);
    }

    // Update lat recieved time.
    LastMessageRecievedTime = MessageStream->GetLastActivityTime();

    // Keep authentication token alive while client is..
    Service->RefreshAuthToken(AuthToken);

    return false;
}

bool GameClient::HandleMessage(const Frpg2ReliableUdpMessage& Message)
{
    const std::vector<std::shared_ptr<GameManager>>& Managers = Service->GetManagers();
    for (auto& Manager : Managers)
    {
        MessageHandleResult Result = Manager->OnMessageRecieved(this, Message);
        if (Result == MessageHandleResult::Error)
        {
            return true;
        }
        else if (Result == MessageHandleResult::Handled)
        {
            return false;
        }
        else if (Result == MessageHandleResult::Unhandled)
        {
            // Continue;
        }
    }

    return true;
}

std::string GameClient::GetName()
{
    return Connection->GetName();
}