/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatManager.h"
#include "Server/GameService/GameManagers/AntiCheat/DS3_AntiCheatTrigger.h"

#include "Server/GameService/Utils/DS3_GameIds.h"

#include <string>
#include <memory>
#include <unordered_map>

 // Triggers when the clients stats are impossible (below minimums, higher than available based on soul level, etc).

class DS3_AntiCheatTrigger_ImpossibleStats : public DS3_AntiCheatTrigger
{
public:
    DS3_AntiCheatTrigger_ImpossibleStats(DS3_AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance);

    virtual bool Scan(std::shared_ptr<GameClient> client, std::string& extraInfo) override;
    virtual std::string GetName() override;
    virtual float GetPenaltyScore() override;

protected:    
    enum class StatType
    {
        Vigor,
        Attunement,
        Endurance,
        Vitality,
        Strength,
        Dexterity,
        Intelligence,
        Faith,
        Luck
    };

    // Maximum soul level the player can be.
    inline const static size_t k_max_soul_level = 802;

    // Maximum level of an individual stat.
    inline const static size_t k_max_stat_level = 99;

    // How many stats you have at base level.
    inline const static size_t k_level_1_stat_count = 90;

    // see if player has been to all estus shard areas and see if it matches charge count.
    // check base hp/fp/stamina matches stats
    // check if player has visited area for covenant active
    // check for sudden delta increases in souls / soul_level
    // check max weapon level is viable based on area played (been to titanite slab area)
    // check if play-time vs soul level is impossible
    // check valid name
    // decipher RequestUpdatePlayerCharacter data, probably useful for anticheat.
    // check rce.
    // check logs, if getting lots of suspicious items in places like shrine, probably sus

    // The minimum value any stat can be based on the starting classes.
    inline const static std::unordered_map<StatType, size_t> k_min_stat_values = {
        { StatType::Vigor,          9 },
        { StatType::Attunement,     6 },
        { StatType::Endurance,      9 },
        { StatType::Vitality,       7 },
        { StatType::Strength,       7 },
        { StatType::Dexterity,      8 },
        { StatType::Intelligence,   7 },
        { StatType::Faith,          7 },
        { StatType::Luck,           7 }
    };

    using StatAdjustments = std::unordered_map<StatType, int>;
    inline const static std::unordered_map<int, StatAdjustments> k_ItemStatAdjustments = {

        // Prisoner's Chain
        {
            20550, 
            { 
                { StatType::Vigor,          5 },
                { StatType::Endurance,      5 },
                { StatType::Vitality,       5 }
            }
        },

        // Knights Ring,
        {
            20460,
            {
                { StatType::Strength,       5 }
            }
        },

        // Hunters Ring,
        {
            20470,
            {
                { StatType::Dexterity,      5 }
            }
        },

        // Scholars Ring,
        {
            20150,
            {
                { StatType::Intelligence,   5 }
            }
        },

        // Priestess Ring,
        {
            20160,
            {
                { StatType::Faith,          5 }
            }
        },

        // Carthus Milkring
        {
            20450,
            {
                { StatType::Dexterity,      3 }
            }
        }

    };

};