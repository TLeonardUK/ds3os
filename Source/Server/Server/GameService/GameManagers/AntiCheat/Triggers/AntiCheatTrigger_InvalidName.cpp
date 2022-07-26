/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/Triggers/AntiCheatTrigger_InvalidName.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/Server.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"
#include "Config/RuntimeConfig.h"

AntiCheatTrigger_InvalidName::AntiCheatTrigger_InvalidName(AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool AntiCheatTrigger_InvalidName::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    auto& AllStatus = client->GetPlayerState().GetPlayerStatus();
    if (AllStatus.has_player_status())
    {
        std::wstring wideName = BytesToWidestring(AllStatus.player_status().name());

        // Check if length of widestring name is valid.
        if (wideName.size() < k_min_name_length || wideName.size() > k_max_name_length)
        {
            extraInfo = StringFormat("Name '%s' has invalid length of %zi.", WidestringToBytes(wideName), wideName.size());
            return true;
        }
    }

    return false;
}

std::string AntiCheatTrigger_InvalidName::GetName()
{
    return "Invalid Name";
}

float AntiCheatTrigger_InvalidName::GetPenaltyScore()
{
    return ServerInstance->GetConfig().AntiCheatScore_ImpossibleName;
}
