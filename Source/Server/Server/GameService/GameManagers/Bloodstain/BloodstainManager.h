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

// Handles client requests relating to leaving
// and replaying bloodstain ghosts in the world.

class BloodstainManager
    : public GameManager
{
public:    
    BloodstainManager(Server* InServerInstance);

    virtual MessageHandleResult OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    MessageHandleResult Handle_RequestCreateBloodstain(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    MessageHandleResult Handle_RequestGetBloodstainList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);

private:
    Server* ServerInstance;

};