/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ImpossibleStats.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/Utils/DS3_GameIds.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Server.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Config/RuntimeConfig.h"

namespace 
{
    size_t CalculateMinimumLevelUpCost(int SoulLevel)
    {
        // We skip levels below 13, then use a seperate calculation and our starting classes
        // will bias the costs. So the result may be a little lower than normal.

        if (SoulLevel < 13)
        {
            return 0;
        }

        size_t Total = 0;

        size_t SoulLevel2 = SoulLevel * SoulLevel;
        size_t SoulLevel3 = SoulLevel * SoulLevel * SoulLevel;

        for (size_t i = 13; i <= SoulLevel; i++)
        {
            double LevelCost = (0.02 * SoulLevel3 + 3.06 * SoulLevel2 + 105.6 * SoulLevel - 895);
            Total += static_cast<size_t>(LevelCost);
        }

        return Total;
    }
};

DS3_AntiCheatTrigger_ImpossibleStats::DS3_AntiCheatTrigger_ImpossibleStats(DS3_AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : DS3_AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool DS3_AntiCheatTrigger_ImpossibleStats::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    auto& AllStatus = client->GetPlayerStateType<DS3_PlayerState>().GetPlayerStatus();
    if (AllStatus.has_player_status() && AllStatus.has_equipment())
    {
        auto& PlayerStatus = AllStatus.player_status();
        auto& Equipment = AllStatus.equipment();

        // Ensure soul level is inside sane bounds.
        if (PlayerStatus.soul_level() <= 0 || PlayerStatus.soul_level() > k_max_soul_level)
        {
            extraInfo = StringFormat("Soul level of %i above maximum of %i.", PlayerStatus.soul_level(), k_max_soul_level);
            return true;
        }

        std::unordered_map<StatType, int> Stats = {
            { StatType::Vigor,          PlayerStatus.vigor() },
            { StatType::Attunement,     PlayerStatus.attunement() },
            { StatType::Endurance,      PlayerStatus.endurance() },
            { StatType::Vitality,       PlayerStatus.vitality() },
            { StatType::Strength,       PlayerStatus.strength() },
            { StatType::Dexterity,      PlayerStatus.dexterity() },
            { StatType::Intelligence,   PlayerStatus.intelligence() },
            { StatType::Faith,          PlayerStatus.faith() },
            { StatType::Luck,           PlayerStatus.luck() },
        };

        // Adjust stats based on items held so we can the "base stats".
        auto AdjustStats = [&Stats](int id)
        {
            if (auto Iter = k_ItemStatAdjustments.find(id); Iter != k_ItemStatAdjustments.end())
            {
                const StatAdjustments& Adjustments = Iter->second;
                for (auto& Pair : Adjustments)
                {
                    Stats[Pair.first] -= Pair.second;
                }
            }
        };

        AdjustStats(Equipment.ring_1());
        AdjustStats(Equipment.ring_2());
        AdjustStats(Equipment.ring_3());
        AdjustStats(Equipment.ring_4());

        // Check all the stats are inside sane bounds.
        int TotalStats = 0;
        for (auto Pair : Stats)
        {        
            if (Pair.second > k_max_stat_level)
            {
                extraInfo = StringFormat("Stat %i with value of %i is above max stat level %i.", Pair.first, Pair.second, k_max_stat_level);
                return true;
            }

            int MinStatValue = k_min_stat_values.at(Pair.first);
            if (Pair.second < MinStatValue)
            {
                extraInfo = StringFormat("Stat %i has value of %i but minimum is %i.", Pair.first, Pair.second, MinStatValue);
                return true;
            }

            TotalStats += Pair.second;
        }

        // Check its actually possible to acquire this many stats at this level.
        int MaxTotalStats = k_level_1_stat_count + (PlayerStatus.soul_level() - 1);
        if (TotalStats > MaxTotalStats)
        {
            extraInfo = StringFormat("Total of %i stats, but at soul level %i with max possible of %i.", TotalStats, PlayerStatus.soul_level(), MaxTotalStats);
            return true;
        }

        // Ensure soul memory is equal or greater than the total cost to level up to this level.
        // Skip this one for now, it produces too many false positives.
        /*size_t LevelCost = CalculateMinimumLevelUpCost(PlayerStatus.soul_level());
        if (PlayerStatus.soul_memory() < LevelCost)
        {
            extraInfo = StringFormat("Level %i requires %i souls, but only has a memory of %i.", PlayerStatus.soul_level(), LevelCost, PlayerStatus.soul_memory());
            return true;
        }*/

        // Player has more souls than they ever obtained?
        if (PlayerStatus.souls() > PlayerStatus.soul_memory())
        {
            extraInfo = StringFormat("Has %i souls, but only acquired %i.", PlayerStatus.souls(), PlayerStatus.soul_memory());
            return true;            
        }
    }

    return false;
}

std::string DS3_AntiCheatTrigger_ImpossibleStats::GetName()
{
    return "Impossible Stats";
}

float DS3_AntiCheatTrigger_ImpossibleStats::GetPenaltyScore()
{
    return ServerInstance->GetConfig().AntiCheatScore_ImpossibleStats;
}
