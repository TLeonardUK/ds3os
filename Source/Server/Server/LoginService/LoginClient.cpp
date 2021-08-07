// Dark Souls 3 - Open Server

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

#include "Protobuf/Frpg2RequestMessage.pb.h"

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
        Warning("[%s] Client disconnected.", GetName().c_str());
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
        // Login server only accepts RequestQueryLoginServerInfo messages. 
        // Nice and straight forward!
        Frpg2RequestMessage::RequestQueryLoginServerInfo Request;
        if (!Request.ParseFromArray(Message.Payload.data(), Message.Payload.size()))
        {
            Warning("[%s] Disconnecting client as recieved message that could not be parsed into expected format.", GetName().c_str());
            return true;
        }

        Log("[%s] Recieved RequestQueryLoginServerInfo, client is on steam account %s.", GetName().c_str(), Request.steam_id().c_str());
        Frpg2RequestMessage::RequestQueryLoginServerInfoResponse Response;
        // TODO: If on same subnet as servers private ip, return the private ip not the external one.
        const RuntimeConfig& Config = Service->GetServer()->GetConfig();
        Response.set_server_ip(Config.ServerIP);
        Response.set_port(Config.AuthServerPort);

        if (!MessageStream->Send(&Response, Message.Header.request_index))
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