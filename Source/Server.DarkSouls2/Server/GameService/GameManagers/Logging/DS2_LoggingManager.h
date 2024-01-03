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

struct Frpg2ReliableUdpMessage;
class Server;

// Handles telemetry messages sent by the client, usually this logs things
// like item usage, game settngs, match results, etc.

class DS2_LoggingManager
    : public GameManager
{
public:    
    DS2_LoggingManager(Server* InServerInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    MessageHandleResult Handle_RequestNotifyBuyItem(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyDeath(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyDisconnectSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyJoinGuestPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyJoinSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyKillEnemy(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyKillPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyLeaveGuestPlayer(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyLeaveSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyMirrorKnight(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyOfflineDeathCount(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;

};