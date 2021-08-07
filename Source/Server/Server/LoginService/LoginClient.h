// Dark Souls 3 - Open Server

#pragma once

#include <memory>
#include <string>

class LoginService;
class NetConnection;
class Frpg2PacketStream;
class Frpg2MessageStream;
class RSAKeyPair;

// Represents an individual client connected to the login service.

class LoginClient 
{
public:
    LoginClient(LoginService* OwningService, std::shared_ptr<NetConnection> InConnection, RSAKeyPair* InServerRSAKey);

    // If this returns true the client is expected to be disconnected and is disposed of.
    bool Poll();

    std::string GetName();

private:    
    LoginService* Service;

    std::shared_ptr<NetConnection> Connection;
    std::shared_ptr<Frpg2MessageStream> MessageStream;

    double LastMessageRecievedTime = 0.0;

};