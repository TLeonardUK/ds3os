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

// Handles all client requests to do with matchmaking
// Which basically means signs, invasions, summoning, etc.

class MatchmakingManager
    : public GameManager
{
public:    
    MatchmakingManager(Server* InServerInstance);

    virtual bool OnMessageRecieved(GameClient* Client, const Frpg2ReliableUdpMessage& Message) override;

    virtual std::string GetName() override;

protected:
    bool Handle_RequestGetSignList(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    bool Handle_RequestCreateSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);
    bool Handle_RequestRemoveSign(GameClient* Client, const Frpg2ReliableUdpMessage& Message);    

private:
    Server* ServerInstance;

    //std::unordered_map<OnlineAreaId, SummonSign> SignList;

};