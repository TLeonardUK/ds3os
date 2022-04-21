/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_ImpossibleStats.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Server.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"
#include "Config/RuntimeConfig.h"

namespace 
{
    int CalculateMinimumLevelUpCost(int SoulLevel)
    {
        // We skip levels below 13, then use a seperate calculation and our starting classes
        // will bias the costs. So the result may be a little lower than normal.

        if (SoulLevel < 13)
        {
            return 0;
        }

        int Total = 0;

        int SoulLevel2 = SoulLevel * SoulLevel;
        int SoulLevel3 = SoulLevel * SoulLevel * SoulLevel;

        for (int i = 13; i <= SoulLevel; i++)
        {
            float LevelCost = (0.02f * SoulLevel3 + 3.06f * SoulLevel2 + 105.6 * SoulLevel - 895);
            //Log("Level[%i] costs %i", i, LevelCost);
            Total += static_cast<int>(LevelCost);
        }

        return Total;
    }
};

AntiCheatTrigger_ImpossibleStats::AntiCheatTrigger_ImpossibleStats(AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool AntiCheatTrigger_ImpossibleStats::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    auto& AllStatus = client->GetPlayerState().GetPlayerStatus();
    if (AllStatus.has_player_status())
    {
        auto& PlayerStatus = AllStatus.player_status();

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
        int LevelCost = CalculateMinimumLevelUpCost(PlayerStatus.soul_level());
        if (PlayerStatus.soul_memory() < LevelCost)
        {
            extraInfo = StringFormat("Level %i requires %i souls, but only has a memory of %i.", PlayerStatus.soul_level(), LevelCost, PlayerStatus.soul_memory());
            return true;
        }

        // Player has more souls than they ever obtained?
        if (PlayerStatus.souls() > PlayerStatus.soul_memory())
        {
            extraInfo = StringFormat("Has %i souls, but only acquired %i.", PlayerStatus.souls(), LevelCost, PlayerStatus.soul_memory());
            return true;            
        }
    }

    return false;
}

std::string AntiCheatTrigger_ImpossibleStats::GetName()
{
    return "Impossible Stats";
}

float AntiCheatTrigger_ImpossibleStats::GetPenaltyScore()
{
    return ServerInstance->GetConfig().AntiCheatScore_ImpossibleStats;
}
