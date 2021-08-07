// Dark Souls 3 - Open Server

#pragma once

#include "Server/Service.h"

#include <memory>
#include <vector>

class Server;
class LoginClient;
class NetConnection;
class NetConnectionTCP;
class RSAKeyPair;

// The login server is the first server the game client attempts to contact.
// Its responsible for 2 main things:
//      1) Providing the client with first-time-experience information (EULA data etc)
//      2) Providing the client with the address of the authentication server.
// 
// Communication with it is encrypted using a hostname and public key hard-coded 
// into an TEA encrypted block in the game exe. This block is what the loader 
// patches to make the game communicate with different servers.

class LoginService 
    : public Service
{
public:
    LoginService(Server* OwningServer, RSAKeyPair* ServerRSAKey);
    virtual ~LoginService();

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

    std::vector<std::shared_ptr<LoginClient>> Clients;

    RSAKeyPair* ServerRSAKey;

};