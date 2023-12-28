/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatManager.h"
#include "Server/GameService/DarkSouls3/GameManagers/AntiCheat/AntiCheatTrigger.h"

#include <string>
#include <memory>

// Triggers when the clients anti-cheat sends the appropriate flag to notify of cheating.

class AntiCheatTrigger_ClientFlagged : public AntiCheatTrigger
{
public:
    AntiCheatTrigger_ClientFlagged(AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance);

    virtual bool Scan(std::shared_ptr<GameClient> client, std::string& extraInfo) override;
    virtual std::string GetName() override;
    virtual float GetPenaltyScore() override;

};