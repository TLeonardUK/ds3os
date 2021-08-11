/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Service.h"

#include <memory>
#include <vector>

class Server;
class AuthClient;
class NetConnection;
class NetConnectionTCP;
class RSAKeyPair;

// The auth service is accessed by the client from an ip:port provided by 
// the login service. The purpose of the auth service is to calculate a shared
// secret key which will be used for all future communications, as well as 
// provide the client the ip:port of the game service.

class AuthService 
    : public Service
{
public:
    AuthService(Server* OwningServer, RSAKeyPair* ServerRSAKey);
    virtual ~AuthService();

    virtual bool Init() override;
    virtual bool Term() override;
    virtual void Poll() override;

    virtual std::string GetName() override;

    Server* GetServer() { return ServerInstance; }

protected:

    void HandleClientConnection(std::shared_ptr<NetConnection> ClientConnection);

private:
    Server* ServerInstance;

    std::shared_ptr<NetConnectionTCP> Connection;

    std::vector<std::shared_ptr<AuthClient>> Clients;

    RSAKeyPair* ServerRSAKey;

};