/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <memory>

class Server;
class GameService;
class GameClient;
class DS3_AntiCheatManager;

// Base class for all triggers that trip the anti-cheat detection.

class DS3_AntiCheatTrigger
{
public:    

    DS3_AntiCheatTrigger(DS3_AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance);

    // Checks if a given client matches the trigger condition.
    virtual bool Scan(std::shared_ptr<GameClient> client, std::string& extraInfo) = 0;

    // Returns a general description of what this trigger is detecting.
    virtual std::string GetName() = 0;

    // Returns a value applied to the users penalty score if they are detected, when the 
    // users score gets high enough they are penalized.
    virtual float GetPenaltyScore() = 0;

protected:
    DS3_AntiCheatManager* Manager;
    Server* ServerInstance;
    GameService* GameServiceInstance;

};