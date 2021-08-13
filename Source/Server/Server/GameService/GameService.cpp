/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameManagers/Boot/BootManager.h"
#include "Server/GameService/GameManagers/Logging/LoggingManager.h"
#include "Server/GameService/GameManagers/PlayerData/PlayerDataManager.h"
#include "Server/GameService/GameManagers/BloodMessage/BloodMessageManager.h"
#include "Server/GameService/GameManagers/Matchmaking/MatchmakingManager.h"

#include "Server/Server.h"
#include "Server/Streams/Frpg2ReliableUdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Core/Network/NetConnection.h"
#include "Core/Network/NetConnectionUDP.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

GameService::GameService(Server* OwningServer, RSAKeyPair* InServerRSAKey)
    : ServerInstance(OwningServer)
    , ServerRSAKey(InServerRSAKey)
{
    // This list of managers are what actually do the grunt work of the server
    // they recieve and response to messages.
    Managers.push_back(std::make_shared<BootManager>(ServerInstance));
    Managers.push_back(std::make_shared<LoggingManager>(ServerInstance));
    Managers.push_back(std::make_shared<PlayerDataManager>(ServerInstance));
    Managers.push_back(std::make_shared<BloodMessageManager>(ServerInstance));
    Managers.push_back(std::make_shared<MatchmakingManager>(ServerInstance));
}

GameService::~GameService()
{
}

bool GameService::Init()
{
    Connection = std::make_shared<NetConnectionUDP>("Game Service");
    int Port = ServerInstance->GetConfig().GameServerPort;
    if (!Connection->Listen(Port))
    {
        Error("Game service failed to listen on port %i.", Port);
        return false;
    }

    Log("Game service is now listening on port %i.", Port);

    for (auto& Manager : Managers)
    {
        if (!Manager->Init())
        {
            Error("Failed to initialize game manager '%s'", Manager->GetName().c_str());
            return false;
        }
    }

    return true;
}

bool GameService::Term()
{
    for (auto& Manager : Managers)
    {
        if (!Manager->Init())
        {
            Error("Failed to terminate game manager '%s'", Manager->GetName().c_str());
            return false;
        }
    }

    return false;
}

void GameService::Poll()
{
    Connection->Pump();

    for (auto& Manager : Managers)
    {
        Manager->Init();
    }

    while (std::shared_ptr<NetConnection> ClientConnection = Connection->Accept())
    {
        HandleClientConnection(ClientConnection);
    }

    for (auto iter = Clients.begin(); iter != Clients.end(); /* empty */)
    {
        std::shared_ptr<GameClient> Client = *iter;

        if (Client->Poll())
        {
            Log("[%s] Disconnecting client connection.", Client->GetName().c_str());
            DisconnectingClients.push_back(Client);

            Client->MessageStream->Disconnect();

            iter = Clients.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    
    for (auto iter = DisconnectingClients.begin(); iter != DisconnectingClients.end(); /* empty */)
    {
        std::shared_ptr<GameClient> Client = *iter;

        Client->Connection->Pump();
        Client->MessageStream->Pump();

        if (Client->MessageStream->GetState() == Frpg2ReliableUdpStreamState::Closed)
        {
            Log("[%s] Client disconnected.", Client->GetName().c_str());

            // Let all managers know this client has been disconnected, they
            // may need to clean things up.
            for (auto& Manager : Managers)
            {
                Manager->OnClientDisconnected(Client.get());
            }

            iter = Clients.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    // Remove authentication states that have timed out.
    for (auto iter = AuthenticationStates.begin(); iter != AuthenticationStates.end(); /* empty */)
    {
        auto& Pair = *iter;

        float ElapsedTime = GetSeconds() - Pair.second.LastRefreshTime;
        if (ElapsedTime > BuildConfig::AUTH_TICKET_TIMEOUT)
        {
            Log("Authentication token 0x%016llx has expired.", Pair.second.AuthToken);
            iter = AuthenticationStates.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void GameService::HandleClientConnection(std::shared_ptr<NetConnection> ClientConnection)
{
    uint64_t AuthToken;
    int BytesRecieved = 0;

    std::vector<uint8_t> Buffer;
    Buffer.resize(sizeof(uint64_t));
    if (!ClientConnection->Peek(Buffer, 0, sizeof(AuthToken), BytesRecieved) || BytesRecieved != sizeof(AuthToken))
    {
        Log("[%s] Failed to peek authentication token, or not enough data available. Ignoring connection.", ClientConnection->GetName().c_str());
        return;
    }

    AuthToken = *reinterpret_cast<uint64_t*>(Buffer.data());

    Log("[%s] Client connected.", ClientConnection->GetName().c_str());

    // Check we have an authentication state for this client.
    auto AuthStateIter = AuthenticationStates.find(AuthToken);
    if (AuthStateIter == AuthenticationStates.end())
    {
        Log("[%s] Clients authentication token does not appear to be valid. Ignoring connection.", ClientConnection->GetName().c_str());
        return;
    }

    GameClientAuthenticationState& AuthState = (*AuthStateIter).second;

    //Log("[%s] Client will use cipher key %s", ClientConnection->GetName().c_str(), BytesToHex(AuthState.CwcKey).c_str());

    std::shared_ptr<GameClient> Client = std::make_shared<GameClient>(this, ClientConnection, AuthState.CwcKey, AuthState.AuthToken);
    Clients.push_back(Client);

    // Let all managers know this client connected.
    for (auto& Manager : Managers)
    {
        Manager->OnClientConnected(Client.get());
    }
}

std::string GameService::GetName()
{
    return "Game";
}

void GameService::CreateAuthToken(uint64_t AuthToken, const std::vector<uint8_t>& CwcKey)
{
    //Log("[%s] Created authentication token 0x%016llx", Connection->GetName().c_str(), AuthToken);

    GameClientAuthenticationState AuthState;
    AuthState.AuthToken = AuthToken;
    AuthState.CwcKey = CwcKey;
    AuthState.LastRefreshTime = GetSeconds();
    AuthenticationStates.insert({ AuthToken, AuthState });
}

void GameService::RefreshAuthToken(uint64_t AuthToken)
{
    auto AuthStateIter = AuthenticationStates.find(AuthToken);
    if (AuthStateIter == AuthenticationStates.end())
    {
        return;
    }

    AuthStateIter->second.LastRefreshTime = GetSeconds();
}