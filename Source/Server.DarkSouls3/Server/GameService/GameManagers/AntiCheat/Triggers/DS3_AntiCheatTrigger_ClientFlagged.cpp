/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/GameManagers/AntiCheat/Triggers/DS3_AntiCheatTrigger_ClientFlagged.h"
#include "Server/GameService/DS3_PlayerState.h"
#include "Server/GameService/GameClient.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/Utils/DS3_GameIds.h"
#include "Server/Server.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Config/RuntimeConfig.h"

DS3_AntiCheatTrigger_ClientFlagged::DS3_AntiCheatTrigger_ClientFlagged(DS3_AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : DS3_AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool DS3_AntiCheatTrigger_ClientFlagged::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    auto& AllStatus = client->GetPlayerStateType<DS3_PlayerState>().GetPlayerStatus();
    if (AllStatus.has_player_status())
    {
        auto& PlayerStatus = AllStatus.player_status();
        for (int i = 0; i < PlayerStatus.anticheat_data_size(); i++)
        {
            int Flag = PlayerStatus.anticheat_data(i);
            if (Flag == 0x1770)
            {
                return true;
            }
        }
    }

    return false;
}

std::string DS3_AntiCheatTrigger_ClientFlagged::GetName()
{
    return "Client Flagged";
}

float DS3_AntiCheatTrigger_ClientFlagged::GetPenaltyScore()
{
    return ServerInstance->GetConfig().AntiCheatScore_ClientFlagged;
}
