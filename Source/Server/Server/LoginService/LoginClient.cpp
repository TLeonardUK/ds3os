/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/LoginService/LoginClient.h"
#include "Server/LoginService/LoginService.h"
#include "Server/Streams/Frpg2MessageStream.h"
#include "Server/Streams/Frpg2Message.h"

#include "Server/Server.h"

#include "Shared/Platform/Platform.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/SharedProtobufs.h"

LoginClient::LoginClient(LoginService* OwningService, std::shared_ptr<NetConnection> InConnection, RSAKeyPair* InServerRSAKey)
    : Service(OwningService)
    , Connection(InConnection)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2MessageStream>(InConnection, InServerRSAKey);
}

bool LoginClient::Poll()
{
    // Has this client timed out?
    double TimeSinceLastMessage = GetSeconds() - LastMessageRecievedTime;
    if (TimeSinceLastMessage >= BuildConfig::CLIENT_TIMEOUT)
    {
        WarningS(GetName().c_str(), "Client timed out.");
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
        WarningS(GetName().c_str(), "Disconnecting client as message stream was in an error state.");
        return true;
    }

    Frpg2Message Message;
    while (MessageStream->Recieve(&Message))
    {
        if (Message.Header.msg_type != Frpg2MessageType::RequestQueryLoginServerInfo)
        {
            WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected packet type while expecting.");
            return true;
        }

        // Login server only accepts RequestQueryLoginServerInfo messages. 
        // Nice and straight forward!
        Shared_Frpg2RequestMessage::RequestQueryLoginServerInfo Request;
        if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
        {
            WarningS(GetName().c_str(), "Disconnecting client as recieved message that could not be parsed into expected format.");
            return true;
        }

        const RuntimeConfig& Config = Service->GetServer()->GetConfig();
        std::string ServerIP = Service->GetServer()->GetPublicIP().ToString();

        // If user IP is on a private network, we can assume they are on our LAN
        // and return our internal IP address.
        if (Connection->GetAddress().IsPrivateNetwork())
        {
            ServerIP = Service->GetServer()->GetPrivateIP().ToString();
            LogS(GetName().c_str(), "Directing login client to our private ip (%s) as appears to be on private subnet.", ServerIP.c_str());
        }

        Shared_Frpg2RequestMessage::RequestQueryLoginServerInfoResponse Response;
        Response.set_server_ip(ServerIP);
        Response.set_port(Config.AuthServerPort);

        if (!MessageStream->Send(&Response, Frpg2MessageType::Reply, Message.Header.msg_index))
        {
            WarningS(GetName().c_str(), "Disconnecting client as failed to send RequestQueryLoginServerInfo response.");
            return true;
        }

        LastMessageRecievedTime = GetSeconds();
    }

    return false;
}

std::string LoginClient::GetName()
{
    return Connection->GetName();
}