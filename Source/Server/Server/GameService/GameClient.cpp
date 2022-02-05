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
#include "Core/Utils/Strings.h"
#include "Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/Protobufs.h"

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
        WarningS(GetName().c_str(), "Client timed out.");
        return true;
    }

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
                WarningS(GetName().c_str(), "Failed to handle message, ignoring and hoping nothing breaks ...");
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
    Frpg2RequestMessage::ManagementTextMessage Message;
    Message.set_push_message_id(Frpg2RequestMessage::PushID_ManagementTextMessage);
    Message.set_message(TextMessage);
    Message.set_unknown_4(0);
    Message.set_unknown_5(0);

    // Date makes no difference, just hard-code for now.
    Frpg2PlayerData::DateTime* DateTime = Message.mutable_timestamp();
    DateTime->set_year(2021);
    DateTime->set_month(1);
    DateTime->set_day(1);
    DateTime->set_hours(0);
    DateTime->set_minutes(0);
    DateTime->set_seconds(0);
    DateTime->set_tzdiff(0);

    if (!MessageStream->Send(&Message))
    {
        WarningS(GetName().c_str(), "Failed to send game client text message.");
    }
}