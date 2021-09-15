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

#include "Platform/Platform.h"

#include "Core/Utils/Logging.h"
#include "Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Protobuf/Protobufs.h"

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
        Log("[%s] Client disconnected.", GetName().c_str());
        return true;
    }

    // Pump the message stream and handle any messages that come in.
    if (MessageStream->Pump())
    {
        Warning("[%s] Disconnecting client as message stream was in an error state.", GetName().c_str());
        return true;
    }

    Frpg2Message Message;
    while (MessageStream->Recieve(&Message))
    {
        if (Message.Header.msg_type != Frpg2MessageType::RequestQueryLoginServerInfo)
        {
            Warning("[%s] Disconnecting client as recieved unexpected packet type while expecting.", GetName().c_str());
            return true;
        }

        // Login server only accepts RequestQueryLoginServerInfo messages. 
        // Nice and straight forward!
        Frpg2RequestMessage::RequestQueryLoginServerInfo Request;
        if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
        {
            Warning("[%s] Disconnecting client as recieved message that could not be parsed into expected format.", GetName().c_str());
            return true;
        }

        //Log("[%s] Recieved RequestQueryLoginServerInfo, client is on steam account %s.", GetName().c_str(), Request.steam_id().c_str());

        const RuntimeConfig& Config = Service->GetServer()->GetConfig();
        std::string ServerIP = Service->GetServer()->GetPublicIP().ToString();

        // If user IP is on a private network, we can assume they are on our LAN
        // and return our internal IP address.
        if (Connection->GetAddress().IsPrivateNetwork())
        {
            Log("[%s] Directing login client to our private ip as appears to be on private subnet.", GetName().c_str());
            ServerIP = Service->GetServer()->GetPrivateIP().ToString();
        }

        Frpg2RequestMessage::RequestQueryLoginServerInfoResponse Response;
        Response.set_server_ip(ServerIP);
        Response.set_port(Config.AuthServerPort);

        if (!MessageStream->Send(&Response, Frpg2MessageType::Reply, Message.Header.msg_index))
        {
            Warning("[%s] Disconnecting client as failed to send RequestQueryLoginServerInfo response.", GetName().c_str());
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