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
#include "Protobuf/DS3_Protobufs.h"

#include <memory>

struct Frpg2ReliableUdpMessage;
class Server;
class GameService;
class DS3_AntiCheatTrigger;

// Snoops player state changes and flags anyone appearing to be cheating.

class DS3_AntiCheatManager
    : public GameManager
{
public:    
    DS3_AntiCheatManager(Server* InServerInstance, GameService* InGameServiceInstance);

    virtual void Poll() override;
    
    virtual std::string GetName() override;

private:
    Server* ServerInstance;
    GameService* GameServiceInstance;

    std::vector<std::shared_ptr<DS3_AntiCheatTrigger>> Triggers;

};