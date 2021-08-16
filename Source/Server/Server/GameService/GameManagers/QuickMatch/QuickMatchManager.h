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

// Handles client requests for joining/leaving quick matches (undead matches)

class QuickMatchManager
    : public GameManager
{
public:    
    QuickMatchManager(Server* InServerInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    MessageHandleResult Handle_RequestSearchQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUnregisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestUpdateQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestJoinQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestAcceptQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRejectQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestRegisterQuickMatch(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestSendQuickMatchStart(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestSendQuickMatchResult(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;

};