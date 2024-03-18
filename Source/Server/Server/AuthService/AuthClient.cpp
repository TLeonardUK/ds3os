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

#include "Shared/Platform/Platform.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Random.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

#include "Shared/Core/Crypto/CWCCipher.h"

#include "Protobuf/SharedProtobufs.h"

#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

AuthClient::AuthClient(AuthService* OwningService, std::shared_ptr<NetConnection> InConnection, RSAKeyPair* InServerRSAKey)
    : Service(OwningService)
    , Connection(InConnection)
{
    LastMessageRecievedTime = GetSeconds();

    MessageStream = std::make_shared<Frpg2MessageStream>(InConnection, InServerRSAKey);
}

bool AuthClient::Poll()
{
    GameTypeConfig& GameConfig = BuildConfig::GameConfig[(int)Service->GetServer()->GetGameType()];

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
        Warning(GetName().c_str(), "Disconnecting client as connection was in an error state.");
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
    
    switch (State)
    {
    // Waiting for initial handshake request.
    case AuthClientState::WaitingForHandshakeRequest:
        {
            Frpg2Message Message;
            if (MessageStream->Recieve(&Message))
            {
                if (Message.Header.msg_type != Frpg2MessageType::RequestHandshake)
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected packet type (%i) while expected RequestHandshake.", Message.Header.msg_type);
                    return true;
                }

                // First request is always the handshake request. 
                Shared_Frpg2RequestMessage::RequestHandshake Request;
                if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected message, expecting RequestHandshake.");
                    return true;
                }

                // Covert the CWC key to a byte buffer.
                std::string string = Request.aes_cwc_key();
                uint8_t* string_ptr = reinterpret_cast<uint8_t*>(string.data());

                CwcKey.assign(string_ptr, string_ptr + string.length());

                // Disable cipher while we send this "hardcoded" message.
                MessageStream->SetCipher(nullptr, nullptr);
                
                // TODO: Not sure whats going on with this payload right now. 
                // Seems to be 11 bytes followed by 16 zeros, unencrypted. Key exchange of some description?
                Frpg2Message Response;
                Response.Payload.resize(27);
                FillRandomBytes(Response.Payload);
                memset(Response.Payload.data() + 11, 0, 16);

                if (!MessageStream->Send(Response, Frpg2MessageType::Reply, Message.Header.msg_index))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as failed to send cipher validation response.");
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
                if (Message.Header.msg_type != Frpg2MessageType::GetServiceStatus)
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected packet type while expected GetServiceStatus.");
                    return true;
                }

                Shared_Frpg2RequestMessage::GetServiceStatus Request;
                if (!Request.ParseFromArray(Message.Payload.data(), (int)Message.Payload.size()))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected message, expecting GetServiceStatus.");
                    return true;
                }

                SteamId = Request.steam_id();

                // Note: I think empty response is sent back here if an update is available.

                Shared_Frpg2RequestMessage::GetServiceStatusResponse Response;
                if (Request.app_version() >= GameConfig.MIN_APP_VERSION && Request.app_version() <= GameConfig.APP_VERSION)
                {
                    Response.set_id(2);
                    Response.set_steam_id("\0");
                    Response.set_unknown_1(0);
                    Response.set_app_version(Request.app_version());
                }
                else
                {
                    WarningS(GetName().c_str(), "Disconnecting client as they have unsupported version: %zi, highest we support is %i.", Request.app_version(), GameConfig.APP_VERSION);
                }

                if (!MessageStream->Send(&Response, Frpg2MessageType::Reply, Message.Header.msg_index))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as failed to send GetServiceStatusResponse response.");
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
                if (Message.Header.msg_type != Frpg2MessageType::KeyMaterial)
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected packet type while expected key material.");
                    return true;
                }
                if (Message.Payload.size() != 8)
                {
                    WarningS(GetName().c_str(), "Disconnecting client as key exchange payload was different size to expected.");
                    return true;
                }

                // This is our authentication key for the game session.
                Frpg2Message KeyResponse;
                KeyResponse.Payload.resize(16);
                // Lower 8 bytes are what the client sent, upper 8 bytes are random data we fill in.
                FillRandomBytes(KeyResponse.Payload); 
                memcpy(KeyResponse.Payload.data(), Message.Payload.data(), 8);

                GameCwcKey = KeyResponse.Payload;

                if (!MessageStream->Send(KeyResponse, Frpg2MessageType::Reply, Message.Header.msg_index))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as failed to send key exchange.");
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
                const RuntimeConfig& RuntimeConfig = Service->GetServer()->GetConfig();
                std::string ServerIP = Service->GetServer()->GetPublicIP().ToString();

                // Format Note:
                // The message payload is stored as:
                //      Bytes 0-15: GameCwcKey Calculated Above
                //      Bytes 16- : SteamTicket as recieved from GetAuthSessionTicket

                if (Message.Header.msg_type != Frpg2MessageType::SteamTicket)
                {
                    WarningS(GetName().c_str(), "Disconnecting client as recieved unexpected packet type while expected steam ticket.");
                    return true;
                }

                // Validate the steam ticket.
                std::vector<uint8_t> Ticket;
                Ticket.assign(Message.Payload.data() + 16, Message.Payload.data() + 16 + (Message.Payload.size() - 16));

                uint64 SteamIdInt;
                sscanf(SteamId.c_str(), "%016llx", &SteamIdInt);
                CSteamID SteamIdStruct(SteamIdInt);                

                if constexpr (BuildConfig::AUTH_ENABLED)
                {
                    double Start = GetHighResolutionSeconds();

                    int AuthResult = SteamGameServer()->BeginAuthSession(Ticket.data(), (int)Ticket.size(), SteamIdStruct);
                    SteamGameServer()->EndAuthSession(SteamIdStruct);

                    double Elapsed = GetHighResolutionSeconds() - Start;

                    if (AuthResult != k_EBeginAuthSessionResultOK)
                    {
                        WarningS(GetName().c_str(), "Disconnecting client as steam ticket authentication failed with error %i.", AuthResult);
                        return true;
                    }
                    else
                    {
                        LogS(GetName().c_str(), "Client steam ticket authenticated successfully in %.2f seconds.");
                    }
                }

                // If user IP is on a private network, we can assume they are on our LAN
                // and return our internal IP address.
                if (Connection->GetAddress().IsPrivateNetwork())
                {
                    ServerIP = Service->GetServer()->GetPrivateIP().ToString();
                    LogS(GetName().c_str(), "Directing auth client to our private ip (%s) as appears to be on private subnet.", ServerIP.c_str());
                }

                Frpg2GameServerInfo GameInfo;
                memset(GameInfo.stack_data, 0, sizeof(GameInfo.stack_data));
                memset(GameInfo.game_server_ip, 0, sizeof(GameInfo.game_server_ip));
                FillRandomBytes((uint8_t*)&GameInfo.auth_token, 8);
                memcpy(GameInfo.game_server_ip, ServerIP.data(), ServerIP.size() + 1);
                GameInfo.game_port = RuntimeConfig.GameServerPort;
                GameInfo.SwapEndian();

                Frpg2Message Response;
                Response.Payload.resize(sizeof(GameInfo));
                memcpy(Response.Payload.data(), &GameInfo, sizeof(GameInfo));

                if (!MessageStream->Send(Response, Frpg2MessageType::Reply, Message.Header.msg_index))
                {
                    WarningS(GetName().c_str(), "Disconnecting client as failed to game server info.");
                    return true;
                }

                // Store authentication state in game service.
                std::shared_ptr<GameService> GameServiceInstance = Service->GetServer()->GetService<GameService>();
                GameServiceInstance->CreateAuthToken(GameInfo.auth_token, GameCwcKey);

                LastMessageRecievedTime = GetSeconds();

                VerboseS(GetName().c_str(), "Authentication complete.");
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
