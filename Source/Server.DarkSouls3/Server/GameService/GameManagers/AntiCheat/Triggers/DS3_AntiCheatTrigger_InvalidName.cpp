/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_InvalidName.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Server.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Config/RuntimeConfig.h"

DS3_AntiCheatTrigger_InvalidName::DS3_AntiCheatTrigger_InvalidName(DS3_AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : DS3_AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool DS3_AntiCheatTrigger_InvalidName::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    auto& AllStatus = client->GetPlayerStateType<DS3_PlayerState>().GetPlayerStatus();
    if (AllStatus.has_player_status())
    {
        std::string name = TrimString(AllStatus.player_status().name());

        // Check trimmed length of name is valid.
        if (name.size() < k_min_name_length || name.size() > k_max_name_length)
        {
            extraInfo = StringFormat("Name '%s' has invalid length of %zi.", name.c_str(), name.size());
            return true;
        }
    }

    return false;
}

std::string DS3_AntiCheatTrigger_InvalidName::GetName()
{
    return "Invalid Name";
}

float DS3_AntiCheatTrigger_InvalidName::GetPenaltyScore()
{
    return ServerInstance->GetConfig().AntiCheatScore_ImpossibleName;
}
