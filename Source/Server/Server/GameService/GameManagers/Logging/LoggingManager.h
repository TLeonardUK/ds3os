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

class LoggingManager
    : public GameManager
{
public:    
    LoggingManager(Server* InServerInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    MessageHandleResult Handle_RequestNotifyProtoBufLog(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyKillEnemy(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

    MessageHandleResult Handle_RequestNotifyDisconnectSession(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyRegisterCharacter(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyDie(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyKillBoss(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyJoinMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyLeaveMultiplay(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifySummonSignResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyCreateSignResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestNotifyBreakInResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    
private:
    Server* ServerInstance;

};