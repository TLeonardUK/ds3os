// Dark Souls 3 - Open Server

#pragma once

#include <memory>
#include <string>
#include <vector>

class GameService;
class NetConnection;
class Frpg2ReliableUdpPacketStream;
class RSAKeyPair;
class Cipher;

// Represents an individual client connected to the game service.

class GameClient 
{
public:
    GameClient(GameService* OwningService, std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // If this returns true the client is expected to be disconnected and is disposed of.
    bool Poll();

    std::string GetName();

private:    
    GameService* Service;

    std::shared_ptr<NetConnection> Connection;
    std::shared_ptr<Frpg2ReliableUdpPacketStream> MessageStream;

    uint64_t AuthToken;

    double LastMessageRecievedTime = 0.0;

};