/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/LoginService/LoginService.h"
#include "Server/LoginService/LoginClient.h"

#include "Server/Server.h"

#include "Core/Network/NetConnection.h"
#include "Core/Network/NetConnectionTCP.h"
#include "Core/Utils/Logging.h"

#include "Config/BuildConfig.h"
#include "Config/RuntimeConfig.h"

LoginService::LoginService(Server* OwningServer, RSAKeyPair* InServerRSAKey)
    : ServerInstance(OwningServer)
    , ServerRSAKey(InServerRSAKey)
{
}

LoginService::~LoginService()
{
}

bool LoginService::Init()
{
    Connection = std::make_shared<NetConnectionTCP>("Login Service");
    int Port = ServerInstance->GetConfig().LoginServerPort;
    if (!Connection->Listen(Port))
    {
        LogError("Login service failed to listen on port %i.", Port);
        return false;
    }

    Log("Login service is now listening on port %i.", Port);

    return true;
}

bool LoginService::Term()
{
    return true;
}

void LoginService::Poll()
{
    Connection->Pump();

    while (std::shared_ptr<NetConnection> ClientConnection = Connection->Accept())
    {
        HandleClientConnection(ClientConnection);
    }

    for (auto iter = Clients.begin(); iter != Clients.end(); /* empty */)
    {
        std::shared_ptr<LoginClient> Client = *iter;

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

void LoginService::HandleClientConnection(std::shared_ptr<NetConnection> ClientConnection)
{
    LogS(ClientConnection->GetName().c_str(), "Client connected.");

    std::shared_ptr<LoginClient> Client = std::make_shared<LoginClient>(this, ClientConnection, ServerRSAKey);
    Clients.push_back(Client);
}

std::string LoginService::GetName()
{
    return "Login";
}

