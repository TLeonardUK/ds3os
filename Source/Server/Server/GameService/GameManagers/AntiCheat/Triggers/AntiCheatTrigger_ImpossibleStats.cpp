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

AntiCheatTrigger_ImpossibleStats::AntiCheatTrigger_ImpossibleStats(AntiCheatManager* InCheatManager, Server* InServerInstance, GameService* InGameServiceInstance)
    : AntiCheatTrigger(InCheatManager, InServerInstance, InGameServiceInstance)
{
}

bool AntiCheatTrigger_ImpossibleStats::Scan(std::shared_ptr<GameClient> client, std::string& extraInfo)
{
    // TODO
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
