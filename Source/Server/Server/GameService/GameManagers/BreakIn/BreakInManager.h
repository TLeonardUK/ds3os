/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/GameManager.h"
#include "Protobuf/Protobufs.h"

#include <memory>

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;

// Handles client requests for invading other games.

class BreakInManager
    : public GameManager
{
public:    
    BreakInManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

    virtual void OnLostPlayer(GameClient* Client) override;

protected:
    static bool CanMatchWith(const Frpg2RequestMessage::MatchingParameter& Client, const std::shared_ptr<GameClient>& Match);

    MessageHandleResult Handle_RequestGetBreakInTargetList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectBreakInTarget(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

};