/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/WebUIService/Handlers/SettingsHandler.h"
#include "Server/Server.h"
#include "Server/GameService/GameService.h"
#include "Server/GameService/GameClient.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include <ctime>

SettingsHandler::SettingsHandler(WebUIService* InService)
    : WebUIHandler(InService)
{
} 

void SettingsHandler::Register(CivetServer* Server)
{
    Server->addHandler("/settings", this);
}

bool SettingsHandler::handleGet(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    RuntimeConfig& Config = Service->GetServer()->GetMutableConfig();

    nlohmann::json json;
    json["serverName"] = Config.ServerName;
    json["serverDescription"] = Config.ServerDescription;
    json["password"] = Config.Password;
    json["publicHostname"] = Config.ServerHostname;
    json["privateHostname"] = Config.ServerPrivateHostname;
    json["advertise"] = Config.Advertise;
    json["disableCoop"] = Config.DisableCoop;
    json["disableBloodMessages"] = Config.DisableBloodMessages;
    json["disableBloodStains"] = Config.DisableBloodStains;
    json["disableGhosts"] = Config.DisableGhosts;
    json["disableInvasions"] = Config.DisableInvasions;
    json["disableAutoSummonCoop"] = Config.DisableCoopAutoSummon;
    json["disableAutoSummonInvasions"] = Config.DisableInvasionAutoSummon;
    json["disableWeaponLevelMatching"] = IsWeaponLevelMatchingDisabled();
    json["disableSoulLevelMatching"] = IsSoulLevelMatchingDisabled();
    json["disableSoulMemoryMatching"] = IsSoulMemoryMatchingDisabled();
    json["ignoreInvasionAreaFilter"] = Config.IgnoreInvasionAreaFilter;
    json["antiCheatEnabled"] = Config.AntiCheatEnabled;
    
    RespondJson(Connection, json);

    return true;
}

bool SettingsHandler::handlePost(CivetServer* Server, struct mg_connection* Connection)
{
    if (!Service->IsAuthenticated(Connection))
    {
        mg_send_http_error(Connection, 401, "Token invalid.");
        return true;
    }

    nlohmann::json json;
    if (!ReadJson(Server, Connection, json))
    {
        mg_send_http_error(Connection, 400, "Malformed body.");
        return true;
    }

    RuntimeConfig& Config = Service->GetServer()->GetMutableConfig();

    if (json.contains("serverName"))
    {
        Config.ServerName = json["serverName"];
    }
    if (json.contains("serverDescription"))
    {
        Config.ServerDescription = json["serverDescription"];
    }
    if (json.contains("password"))
    {
        Config.Password = json["password"];
    }

    if (Service->GetServer()->IsDefaultServer())
    {
        if (json.contains("publicHostname"))
        {
            Config.ServerHostname = json["publicHostname"];
        }
        if (json.contains("privateHostname"))
        {
            Config.ServerPrivateHostname = json["privateHostname"];
        }
    }

    if (json.contains("advertise"))
    {
        Config.Advertise = json["advertise"];
    }
    if (json.contains("disableCoop"))
    {
        Config.DisableCoop = json["disableCoop"];
    }
    if (json.contains("disableInvasions"))
    {
        Config.DisableInvasions = json["disableInvasions"];
    }
    if (json.contains("disableBloodMessages"))
    {
        Config.DisableBloodMessages = json["disableBloodMessages"];
    }
    if (json.contains("disableBloodStains"))
    {
        Config.DisableBloodStains = json["disableBloodStains"];
    }
    if (json.contains("disableGhosts"))
    {
        Config.DisableGhosts = json["disableGhosts"];
    }
    if (json.contains("disableAutoSummonCoop"))
    {
        Config.DisableCoopAutoSummon = json["disableAutoSummonCoop"];
    }
    if (json.contains("disableAutoSummonInvasions"))
    {
        Config.DisableInvasionAutoSummon = json["disableAutoSummonInvasions"];
    }
    if (json.contains("ignoreInvasionAreaFilter"))
    {
        Config.IgnoreInvasionAreaFilter = json["ignoreInvasionAreaFilter"];
    }
    if (json.contains("antiCheatEnabled"))
    {
        Config.AntiCheatEnabled = json["antiCheatEnabled"];
    }
    if (json.contains("disableWeaponLevelMatching"))
    {
        if (json["disableWeaponLevelMatching"] != IsWeaponLevelMatchingDisabled())
        {
            Config.SummonSignMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
            Config.WayOfBlueMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
            Config.DarkSpiritInvasionMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
            Config.MoundMakerInvasionMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
            Config.CovenantInvasionMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
            Config.UndeadMatchMatchingParameters.DisableWeaponLevelMatching = json["disableWeaponLevelMatching"];
        }
    }
    if (json.contains("disableSoulLevelMatching"))
    {
        if (json["disableSoulLevelMatching"] != IsSoulLevelMatchingDisabled())
        {
            Config.SummonSignMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
            Config.WayOfBlueMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
            Config.DarkSpiritInvasionMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
            Config.MoundMakerInvasionMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
            Config.CovenantInvasionMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
            Config.UndeadMatchMatchingParameters.DisableLevelMatching = json["disableSoulLevelMatching"];
        }
    }
    if (json.contains("disableSoulMemoryMatching"))
    {
        if (json["disableSoulMemoryMatching"] != IsSoulMemoryMatchingDisabled())
        {        
            Config.DS2_WhiteSoapstoneMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_SmallWhiteSoapstoneMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_RedSoapstoneMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_MirrorKnightMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_DragonEyeMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_RedEyeOrbMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_BlueEyeOrbMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_BellKeeperMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_RatMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_BlueSentinelMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
            Config.DS2_ArenaMatchingParameters.DisableSoulMemoryMatching = json["disableSoulMemoryMatching"];
        }
    }

    Service->GetServer()->SaveConfig();

    LogS("WebUI", "Settings were updated.");

    RespondJson(Connection, json);

    return true;
}

bool SettingsHandler::IsWeaponLevelMatchingDisabled()
{
    RuntimeConfig& Config = Service->GetServer()->GetMutableConfig();

    return
        Config.SummonSignMatchingParameters.DisableWeaponLevelMatching ||
        Config.WayOfBlueMatchingParameters.DisableWeaponLevelMatching ||
        Config.DarkSpiritInvasionMatchingParameters.DisableWeaponLevelMatching ||
        Config.MoundMakerInvasionMatchingParameters.DisableWeaponLevelMatching ||
        Config.CovenantInvasionMatchingParameters.DisableWeaponLevelMatching ||
        Config.UndeadMatchMatchingParameters.DisableWeaponLevelMatching;
}

bool SettingsHandler::IsSoulLevelMatchingDisabled()
{
    RuntimeConfig& Config = Service->GetServer()->GetMutableConfig();

    return
        Config.SummonSignMatchingParameters.DisableLevelMatching ||
        Config.WayOfBlueMatchingParameters.DisableLevelMatching ||
        Config.DarkSpiritInvasionMatchingParameters.DisableLevelMatching ||
        Config.MoundMakerInvasionMatchingParameters.DisableLevelMatching ||
        Config.CovenantInvasionMatchingParameters.DisableLevelMatching ||
        Config.UndeadMatchMatchingParameters.DisableLevelMatching;
}

bool SettingsHandler::IsSoulMemoryMatchingDisabled()
{
    RuntimeConfig& Config = Service->GetServer()->GetMutableConfig();

    return 
        Config.DS2_WhiteSoapstoneMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_SmallWhiteSoapstoneMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_RedSoapstoneMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_MirrorKnightMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_DragonEyeMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_RedEyeOrbMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_BlueEyeOrbMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_BellKeeperMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_RatMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_BlueSentinelMatchingParameters.DisableSoulMemoryMatching ||
        Config.DS2_ArenaMatchingParameters.DisableSoulMemoryMatching;
}