/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/AuthService/AuthService.h"
#include "Server/AuthService/AuthClient.h"

#include "Server/Server.h"

#include "Core/Network/NetConnection.h"
#include "Core/Network/NetConnectionTCP.h"
#include "Core/Utils/Logging.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

AuthService::AuthService(Server* OwningServer, RSAKeyPair* InServerRSAKey)
    : ServerInstance(OwningServer)
    , ServerRSAKey(InServerRSAKey)
{
}

AuthService::~AuthService()
{
}

bool AuthService::Init()
{
    Connection = std::make_shared<NetConnectionTCP>("Auth Service");
    int Port = ServerInstance->GetConfig().AuthServerPort;
    if (!Connection->Listen(Port))
    {
        Error("Auth service failed to listen on port %i.", Port);
        return false;
    }

    Success("Auth service is now listening on port %i.", Port);

    return true;
}

bool AuthService::Term()
{
    return false;
}

void AuthService::Poll()
{
    Connection->Pump();

    while (std::shared_ptr<NetConnection> ClientConnection = Connection->Accept())
    {
        HandleClientConnection(ClientConnection);
    }

    for (auto iter = Clients.begin(); iter != Clients.end(); /* empty */)
    {
        std::shared_ptr<AuthClient> Client = *iter;

        if (Client->Poll())
        {
            iter = Clients.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void AuthService::HandleClientConnection(std::shared_ptr<NetConnection> ClientConnection)
{
    Log("[%s] Client connected.", ClientConnection->GetName().c_str());

    std::shared_ptr<AuthClient> Client = std::make_shared<AuthClient>(this, ClientConnection, ServerRSAKey);
    Clients.push_back(Client);
}

std::string AuthService::GetName()
{
    return "Auth";
}

