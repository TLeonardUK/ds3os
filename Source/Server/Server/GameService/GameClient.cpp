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

#include "Shared/Platform/Platform.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/SharedProtobufs.h"

GameClient::GameClient(GameService* OwningService, std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& CwcKey, uint64_t InAuthToken)
    : Service(OwningService)
    , Connection(InConnection)
    , AuthToken(InAuthToken)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2ReliableUdpMessageStream>(InConnection, CwcKey, AuthToken, false, &Service->GetServer()->GetGameInterface());

    State = Service->GetServer()->GetGameInterface().CreatePlayerState();
}

bool GameClient::Poll()
{
    // If we've got a delayed disconnect pending, check if we should disconnect them now.
    if (DisconnectTime > 0.0 && GetSeconds() > DisconnectTime)
    {
        WarningS(GetName().c_str(), "Disconnecting client (due to flagged delayed disconnect).");
        return true;
    }

    // Client disconnected.
    if (Connection->Pump())
    {
        WarningS(GetName().c_str(), "Disconnecting client as connection was in an error state.");
        return true;
    }
    if (!Connection->IsConnected())
    {
        LogS(GetName().c_str(), "Client disconnected.");
        return true;
    }

    // Pump the message stream and handle any messages that come in.
    if (MessageStream->Pump())
    {
        WarningS(GetName().c_str(), "Disconnecting client as message stream closed.");
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
                WarningS(GetName().c_str(), "Disconnecting client as failed to handle message.");
                return true;
            }
            else
            {
                WarningS(GetName().c_str(), "Failed to handle message '%s', ignoring and hoping nothing breaks ...", Message.Protobuf ? Message.Protobuf->GetTypeName().c_str() : "Unknown");
            }
        }

        // TODO: Find a better way to do this that doesn't break our abstraction.
        MessageStream->HandledPacket(Message.AckSequenceIndex);
    }

    // Update lat recieved time.
    LastMessageRecievedTime = MessageStream->GetLastActivityTime();

    // Has this client timed out?
    double TimeSinceLastMessage = GetSeconds() - LastMessageRecievedTime;
    if (TimeSinceLastMessage >= BuildConfig::CLIENT_TIMEOUT)
    {
        WarningS(GetName().c_str(), "Client timed out.");
        return true;
    }

    // Keep authentication token alive while client is..
    Service->RefreshAuthToken(AuthToken);

    return false;
}

bool GameClient::HandleMessage(const Frpg2ReliableUdpMessage& Message)
{
    //WarningS(GetName().c_str(), "-> %s", Message.Protobuf->GetTypeName().c_str());

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

void GameClient::SendTextMessage(const std::string& TextMessage)
{
    Service->GetServer()->GetGameInterface().SendManagementMessage(*MessageStream, TextMessage);
}
