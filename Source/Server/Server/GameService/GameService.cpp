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
#include "Server/GameService/GameManager.h"

#include "Server/Server.h"
#include "Server/Streams/Frpg2ReliableUdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"

#include "Shared/Core/Network/NetConnection.h"
#include "Shared/Core/Network/NetConnectionUDP.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DebugObjects.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

GameService::GameService(Server* OwningServer, RSAKeyPair* InServerRSAKey)
    : ServerInstance(OwningServer)
    , ServerRSAKey(InServerRSAKey)
{
}

GameService::~GameService()
{
}

void GameService::RegisterManager(std::shared_ptr<GameManager> Manager)
{
    Managers.push_back(Manager);
}

bool GameService::Init()
{
    ServerInstance->GetGameInterface().RegisterGameManagers(*this);

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

    TrimDatabase();

    return true;
}

bool GameService::Term()
{
    for (auto& Manager : Managers)
    {
        if (!Manager->Term())
        {
            Error("Failed to terminate game manager '%s'", Manager->GetName().c_str());
            return false;
        }
    }

    return true;
}

void GameService::TrimDatabase()
{
    Log("Trimming database entries.");

    for (auto& Manager : Managers)
    {
        Manager->TrimDatabase();
    }

    GetServer()->GetDatabase().Trim();

    NextDatabaseTrim = GetSeconds() + GetServer()->GetConfig().DatabaseTrimInterval;
}

void GameService::Poll()
{
    DebugTimerScope Scope(Debug::GameService_PollTime);

    Connection->Pump();

    for (auto& Manager : Managers)
    {
        Manager->Poll();
    }

    while (std::shared_ptr<NetConnection> ClientConnection = Connection->Accept())
    {
        HandleClientConnection(ClientConnection);
    }

    if (GetSeconds() > NextDatabaseTrim)
    {
        TrimDatabase();
    }

    for (auto iter = Clients.begin(); iter != Clients.end(); /* empty */)
    {
        std::shared_ptr<GameClient> Client = *iter;

        if (Client->Poll())
        {
            LogS(Client->GetName().c_str(), "Disconnecting client connection.");
            DisconnectingClients.push_back(Client);

            Client->MessageStream->Disconnect();

            // Let all managers know this client is being disconnected, they
            // may need to clean things up.
            for (auto& Manager : Managers)
            {
                Manager->OnLostPlayer(Client.get());
            }

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
            LogS(Client->GetName().c_str(), "Client disconnected.");

            iter = DisconnectingClients.erase(iter);
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

        double ElapsedTime = GetSeconds() - Pair.second.LastRefreshTime;
        if (ElapsedTime > BuildConfig::AUTH_TICKET_TIMEOUT)
        {
            Verbose("Authentication token 0x%016llx has expired.", Pair.second.AuthToken);
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
        LogS(ClientConnection->GetName().c_str(), "Failed to peek authentication token, or not enough data available. Ignoring connection.");
        return;
    }

    AuthToken = *reinterpret_cast<uint64_t*>(Buffer.data());

    LogS(ClientConnection->GetName().c_str(), "Client connected.");

    // Check we have an authentication state for this client.
    auto AuthStateIter = AuthenticationStates.find(AuthToken);
    if (AuthStateIter == AuthenticationStates.end())
    {
        LogS(ClientConnection->GetName().c_str(), "Clients authentication token (0x%016llx) does not appear to be valid. Ignoring connection.", AuthToken);
        return;
    }

    GameClientAuthenticationState& AuthState = (*AuthStateIter).second;

    Debug::GameConnections.Add(1);

    std::shared_ptr<GameClient> Client = std::make_shared<GameClient>(this, ClientConnection, AuthState.CwcKey, AuthState.AuthToken);
    Clients.push_back(Client);

    // Let all managers know this client connected.
    for (auto& Manager : Managers)
    {
        Manager->OnGainPlayer(Client.get());
    }
}

std::string GameService::GetName()
{
    return "Game";
}

void GameService::CreateAuthToken(uint64_t AuthToken, const std::vector<uint8_t>& CwcKey)
{
    VerboseS(Connection->GetName().c_str(), "Created authentication token 0x%016llx", AuthToken);

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

std::shared_ptr<GameClient> GameService::FindClientByPlayerId(uint32_t PlayerId)
{
    for (std::shared_ptr<GameClient>& Client : Clients)
    {
        if (Client->GetPlayerState().GetPlayerId() == PlayerId)
        {
            return Client;
        }
    }
    return nullptr;
}

std::shared_ptr<GameClient> GameService::FindClientBySteamId(const std::string& SteamId)
{
    for (std::shared_ptr<GameClient>& Client : Clients)
    {
        if (Client->GetPlayerState().GetSteamId() == SteamId)
        {
            return Client;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<GameClient>> GameService::FindClients(std::function<bool(const std::shared_ptr<GameClient>&)> Predicate)
{
    std::vector<std::shared_ptr<GameClient>> Result;

    for (std::shared_ptr<GameClient>& Client : Clients)
    {
        if (Predicate(Client))
        {
            Result.push_back(Client);
        }
    }

    return Result;
}
