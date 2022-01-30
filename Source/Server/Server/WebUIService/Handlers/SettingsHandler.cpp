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

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

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
    json["disableInvasions"] = Config.DisableInvasions;
    json["disableAutoSummonCoop"] = Config.DisableCoopAutoSummon;
    json["disableAutoSummonInvasions"] = Config.DisableInvasionAutoSummon;
    json["disableWeaponLevelMatching"] = IsWeaponLevelMatchingDisabled();
    json["disableSoulLevelMatching"] = IsSoulLevelMatchingDisabled();
    
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
    if (json.contains("publicHostname"))
    {
        Config.ServerHostname = json["publicHostname"];
    }
    if (json.contains("privateHostname"))
    {
        Config.ServerPrivateHostname = json["privateHostname"];
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
    if (json.contains("disableAutoSummonCoop"))
    {
        Config.DisableCoopAutoSummon = json["disableAutoSummonCoop"];
    }
    if (json.contains("disableAutoSummonInvasions"))
    {
        Config.DisableInvasionAutoSummon = json["disableAutoSummonInvasions"];
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
