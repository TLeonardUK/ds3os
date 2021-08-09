// Dark Souls 3 - Open Server

#pragma once

#include <memory>
#include <string>
#include <vector>

class GameService;
class NetConnection;
class Frpg2ReliableUdpMessageStream;
class Frpg2ReliableUdpMessage;
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

protected:

    //std::shared_ptr<MessageHandlerToken> RegisterMessageHandler(uint32_t MessageHandler, MessageHandlerCallback Callback);

    bool HandleMessage(const Frpg2ReliableUdpMessage& Message);

private:    
    GameService* Service;

    std::shared_ptr<NetConnection> Connection;
    std::shared_ptr<Frpg2ReliableUdpMessageStream> MessageStream;

    uint64_t AuthToken;

    double LastMessageRecievedTime = 0.0;

};