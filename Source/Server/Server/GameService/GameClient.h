/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/PlayerState.h"

#include <memory>
#include <string>
#include <vector>

class GameService;
class NetConnection;
class Frpg2ReliableUdpMessageStream;
struct Frpg2ReliableUdpMessage;
class RSAKeyPair;
class Cipher;
struct SummonSign;

// Represents an individual client connected to the game service.

class GameClient 
    : public std::enable_shared_from_this<GameClient>
{
public:
    GameClient() {}
    GameClient(GameService* OwningService, std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // If this returns true the client is expected to be disconnected and is disposed of.
    bool Poll();

    std::string GetName();

    PlayerState& GetPlayerState() { return *State; }

    template <typename T>
    T& GetPlayerStateType() { return static_cast<T&>(*State); }

    double GetConnectionDuration() { return GetSeconds() - ConnectTime; }

    // Sends a text message displayed at the top of the users screen.
    void SendTextMessage(const std::string& Message);

public:

    std::shared_ptr<NetConnection> Connection;
    std::shared_ptr<Frpg2ReliableUdpMessageStream> MessageStream;

    std::vector<std::shared_ptr<SummonSign>> ActiveSummonSigns;
    std::vector<std::shared_ptr<SummonSign>> ActiveMirrorKnightSummonSigns;    

    double ConnectTime = GetSeconds();

    // When > 0 and current time surpasses it, the client is disconnected.
    double DisconnectTime = 0.0f;

    bool Banned = false;

protected:

    bool HandleMessage(const Frpg2ReliableUdpMessage& Message);

    bool PollInner();

private:    
    GameService* Service;

    uint64_t AuthToken;

    bool IsDisconnecting = false;

    double LastMessageRecievedTime = 0.0;

    std::unique_ptr<PlayerState> State;

};