/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/GameManagers/AntiCheat/AntiCheatManager.h"
#include "Server/GameService/GameManagers/AntiCheat/AntiCheatTrigger.h"

#include <string>
#include <memory>
#include <unordered_map>

 // Triggers when the clients name IGN is invalid.

class AntiCheatTrigger_InvalidName : public AntiCheatTrigger
{
public:
    AntiCheatTrigger_InvalidName(AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance);

    virtual bool Scan(std::shared_ptr<GameClient> client, std::string& extraInfo) override;
    virtual std::string GetName() override;
    virtual float GetPenaltyScore() override;

protected:
    inline const static size_t k_min_name_length = 1;
    inline const static size_t k_max_name_length = 16;

};