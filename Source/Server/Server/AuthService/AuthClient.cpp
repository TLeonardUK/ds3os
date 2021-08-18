/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/AuthService/AuthClient.h"
#include "Server/AuthService/AuthService.h"
#include "Server/GameService/GameService.h"
#include "Server/Streams/Frpg2MessageStream.h"
#include "Server/Streams/Frpg2Message.h"

#include "Server/Server.h"

#include "Platform/Platform.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/Strings.h"
#include "Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Core/Crypto/CWCCipher.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

AuthClient::AuthClient(AuthService* OwningService, std::shared_ptr<NetConnection> InConnection, RSAKeyPair* InServerRSAKey)
    : Service(OwningService)
    , Connection(InConnection)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2MessageStream>(InConnection, InServerRSAKey);
}

bool AuthClient::Poll()
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
    
    switch (State)
    {
    // Waiting for initial handshake request.
    case AuthClientState::WaitingForHandshakeRequest:
        {
            Frpg2Message Message;
            if (MessageStream->Recieve(&Message))
            {
                // First request is always the handshake request. 
                Frpg2RequestMessage::RequestHandshake Request;
                if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
                {
                    Warning("[%s] Disconnecting client as recieved unexpected message, expecting RequestHandshake.", GetName().c_str());
                    return true;
                }

                // Covert the CWC key to a byte buffer.
                std::string string = Request.aes_cwc_key();
                uint8_t* string_ptr = reinterpret_cast<uint8_t*>(string.data());

                CwcKey.assign(string_ptr, string_ptr + string.length());

                //Log("[%s] Recieved handshake request, Key=%s", GetName().c_str(), BytesToHex(CwcKey).c_str());

                // Disable cipher while we send this "hardcoded" message.
                MessageStream->SetCipher(nullptr, nullptr);

                Frpg2Message Response;

                // TODO: Not sure whats going on with this payload right now. 
                // Seems to be 11 bytes followed by 16 zeros, unencrypted. Key exchange of some description?
                Response.Payload.resize(27);
                FillRandomBytes(Response.Payload);
                memset(Response.Payload.data() + 11, 0, 16);

                if (!MessageStream->Send(Response, Message.Header.request_index))
                {
                    Warning("[%s] Disconnecting client as failed to send cipher validation response.", GetName().c_str());
                    return true;
                }

                // Enable new aes-cwc-128 cipher.
                MessageStream->SetCipher(std::make_shared<CWCCipher>(CwcKey), std::make_shared<CWCCipher>(CwcKey)); 

                LastMessageRecievedTime = GetSeconds();
                State = AuthClientState::WaitingForServiceStatusRequest;
            }

            break;
        }

    // Waiting for service status request.
    case AuthClientState::WaitingForServiceStatusRequest:
        {
            Frpg2Message Message;
            if (MessageStream->Recieve(&Message))
            {
                Frpg2RequestMessage::GetServiceStatus Request;
                if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
                {
                    Warning("[%s] Disconnecting client as recieved unexpected message, expecting GetServiceStatus.", GetName().c_str());
                    return true;
                }

                //Log("[%s] Recieved service status request.", GetName().c_str());

                Frpg2RequestMessage::GetServiceStatusResponse Response;
                Response.set_id(2);
                Response.set_steam_id("\0");
                Response.set_unknown_1(0);
                Response.set_app_version(BuildConfig::APP_VERSION);

                if (!MessageStream->Send(&Response, Message.Header.request_index))
                {
                    Warning("[%s] Disconnecting client as failed to send GetServiceStatusResponse response.", GetName().c_str());
                    return true;
                }

                LastMessageRecievedTime = GetSeconds();
                State = AuthClientState::WaitingForKeyData;
            }

            break;
        }

    // Waiting for what looks like shared secret exchange. They 
    // send us x bytes, we append our own data and send back x*2 bytes.
    case AuthClientState::WaitingForKeyData:
        {
            Frpg2Message Message;
            if (MessageStream->Recieve(&Message))
            {
                if (Message.Payload.size() != 8)
                {
                    Warning("[%s] Disconnecting client as key exchange payload was different size to expected.", GetName().c_str());
                    return true;
                }

                // This is our authentication key for the game session.
                Frpg2Message KeyResponse;
                KeyResponse.Payload.resize(16);
                // Lower 8 bytes are what the client sent, upper 8 bytes are random data we fill in.
                FillRandomBytes(KeyResponse.Payload); 
                memcpy(KeyResponse.Payload.data(), Message.Payload.data(), 8);

                //Log("[%s] Recieved key exchange bytes, game session key = %s", GetName().c_str(), BytesToHex(KeyResponse.Payload).c_str());

                // Note: Supposedly the "normal" cwc key should work for the game session - it doesn't seem to though.
                //       This key however does output plaintext, but with a message tag failure. Huuum
                GameCwcKey = KeyResponse.Payload;

                if (!MessageStream->Send(KeyResponse, Message.Header.request_index))
                {
                    Warning("[%s] Disconnecting client as failed to send key exchange.", GetName().c_str());
                    return true;
                }

                LastMessageRecievedTime = GetSeconds();
                State = AuthClientState::WaitingForSteamTicket;
            }
            break;
        }

    // Waiting for user to provide steam ticket.
    case AuthClientState::WaitingForSteamTicket:
        {
            Frpg2Message Message;
            if (MessageStream->Recieve(&Message))
            {
                //Log("[%s] Recieved steam session ticket.", GetName().c_str());

                // TODO: Could actually link to the steamapi libs and authenticate this ticket? 
                //       Not really a big issue tho.

                const RuntimeConfig& RuntimeConfig = Service->GetServer()->GetConfig();

                Frpg2GameServerInfo GameInfo;
                memset(GameInfo.stack_data, 0, sizeof(GameInfo.stack_data));
                memset(GameInfo.game_server_ip, 0, sizeof(GameInfo.game_server_ip));
                FillRandomBytes((uint8_t*)&GameInfo.auth_token, 8);
                memcpy(GameInfo.game_server_ip, RuntimeConfig.ServerIP.data(), RuntimeConfig.ServerIP.size() + 1);
                GameInfo.game_port = RuntimeConfig.GameServerPort;
                GameInfo.SwapEndian();

                Frpg2Message Response;
                Response.Payload.resize(sizeof(GameInfo));
                memcpy(Response.Payload.data(), &GameInfo, sizeof(GameInfo));

                if (!MessageStream->Send(Response, Message.Header.request_index))
                {
                    Warning("[%s] Disconnecting client as failed to game server info.", GetName().c_str());
                    return true;
                }

                // Store authentication state in game service.
                std::shared_ptr<GameService> GameServiceInstance = Service->GetServer()->GetService<GameService>();
                GameServiceInstance->CreateAuthToken(GameInfo.auth_token, GameCwcKey);

                LastMessageRecievedTime = GetSeconds();

                Log("[%s] Authentication complete.", GetName().c_str());
                State = AuthClientState::Complete;
            }
            break;
        }

    // Authentication steps are complete.
    case AuthClientState::Complete:
        {
            // We don't expect anything from the client at this point, just waiting for them to disconnect or timeout.
            break;
        }
    }

    return false;
}

std::string AuthClient::GetName()
{
    return Connection->GetName();
}