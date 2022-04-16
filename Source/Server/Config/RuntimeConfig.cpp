/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Config/RuntimeConfig.h"
#include "Core/Utils/File.h"

#include <cfloat>

bool RuntimeConfig::Save(const std::filesystem::path& Path)
{
    nlohmann::json json;

    Serialize(json, false);

    if (!WriteTextToFile(Path, json.dump(4)))
    {
        return false;
    }

    return true;
}

bool RuntimeConfig::Load(const std::filesystem::path& Path)
{
    std::string JsonText;

    if (!ReadTextFromFile(Path, JsonText))
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(JsonText);
        Serialize(json, true);
    }
    catch (nlohmann::json::parse_error)
    {
        return false;
    }

    return true;
}

template <typename DataType>
void SerializeVar(nlohmann::json& Json, const char* Name, DataType& Value, bool Loading)
{
    if (Loading)
    {
        if (Json.contains(Name))
        {
            Value = Json[Name];
        }
    }
    else
    {
        Json[Name] = Value;
    }
}

template <typename DataType>
void SerializeStructVar(nlohmann::json& Json, const char* Name, DataType& Value, bool Loading)
{
    if (Loading)
    {
        if (Json.contains(Name))
        {
            Value.Serialize(Json[Name], Loading);
        }
    }
    else
    {
        Value.Serialize(Json[Name], Loading);
    }
}

template <typename DataType>
void SerializeVar(nlohmann::json& Json, const char* Name, std::vector<DataType>& Value, bool Loading)
{
    if (Loading)
    {
        if (Json.contains(Name))
        {
            Value.resize(Json[Name].size());

            int Index = 0;
            for (auto& ChildValue : Json[Name].items())
            {
                Value[Index++].Serialize(ChildValue.value(), Loading);
            }
        }
    }
    else
    {
        nlohmann::json Array = nlohmann::json::array();

        for (auto& ChildValue : Value)
        {
            nlohmann::json ChildNode;
            ChildValue.Serialize(ChildNode, Loading);
            Array.push_back(ChildNode);
        }

        Json[Name] = Array;
    }
}

template <>
void SerializeVar(nlohmann::json& Json, const char* Name, std::vector<int>& Value, bool Loading)
{
    if (Loading)
    {
        if (Json.contains(Name))
        {
            Value.resize(Json[Name].size());

            int Index = 0;
            for (auto& ChildValue : Json[Name].items())
            {
                Value[Index++] = ChildValue.value();
            }
        }
    }
    else
    {
        nlohmann::json Array = nlohmann::json::array();

        for (auto& ChildValue : Value)
        {
            Array.push_back(ChildValue);
        }

        Json[Name] = Array;
    }
}

bool RuntimeConfigMatchingParameters::CheckMatch(int HostSoulLevel, int HostWeaponLevel, int ClientSoulLevel, int ClientWeaponLevel, bool HasPassword) const
{
    // Can ignore all of this if we have a password.
    if (PasswordDisablesLimits && HasPassword)
    {
        return true;
    }

    if (!DisableLevelMatching)
    {
        float lower_limit = (HostSoulLevel * LowerLimitMultiplier) + LowerLimitModifier;
        float upper_limit = (HostSoulLevel * UpperLimitMultiplier) + UpperLimitModifier;

        // When host is above 351 upper limit gets removed. 
        if (HostSoulLevel >= RangeRemovalLevel)
        {
            lower_limit = (float)RangeRemovalLevel;
            upper_limit = FLT_MAX;
        }

        // If match falls outside bounds host can't match with them.
        if (ClientSoulLevel < lower_limit || ClientSoulLevel > upper_limit)
        {
            return false;
        }
    }

    if (!DisableWeaponLevelMatching)
    {
        if (ClientWeaponLevel > WeaponLevelUpperLimit[HostWeaponLevel])
        {
            return false;
        }
        if (HostWeaponLevel > WeaponLevelUpperLimit[ClientWeaponLevel])
        {
            return false;
        }
    }

    return true;
}

#define SERIALIZE_VAR(x) SerializeVar(Json, #x, x, Loading);
#define SERIALIZE_STRUCT_VAR(x) SerializeStructVar(Json, #x, x, Loading);

bool RuntimeConfigAnnouncement::Serialize(nlohmann::json& Json, bool Loading)
{
    SERIALIZE_VAR(Header);
    SERIALIZE_VAR(Body);

    return true;
}

bool RuntimeConfigMatchingParameters::Serialize(nlohmann::json& Json, bool Loading)
{
    SERIALIZE_VAR(LowerLimitMultiplier);
    SERIALIZE_VAR(LowerLimitModifier);
    SERIALIZE_VAR(UpperLimitMultiplier);
    SERIALIZE_VAR(UpperLimitModifier);
    SERIALIZE_VAR(WeaponLevelUpperLimit);
    SERIALIZE_VAR(RangeRemovalLevel);
    SERIALIZE_VAR(PasswordDisablesLimits);
    SERIALIZE_VAR(DisableLevelMatching);
    SERIALIZE_VAR(DisableWeaponLevelMatching);
    
    return true;
}

bool RuntimeConfig::Serialize(nlohmann::json& Json, bool Loading)
{
    SERIALIZE_VAR(ServerName);
    SERIALIZE_VAR(ServerDescription);
    SERIALIZE_VAR(ServerHostname);
    SERIALIZE_VAR(ServerPrivateHostname);

    SERIALIZE_VAR(MasterServerIp);
    SERIALIZE_VAR(MasterServerPort);
    SERIALIZE_VAR(Advertise);
    SERIALIZE_VAR(AdvertiseHearbeatTime);
    SERIALIZE_VAR(Password);
    SERIALIZE_VAR(ModsWhitelist);
    SERIALIZE_VAR(ModsBlacklist);
    SERIALIZE_VAR(ModsRequiredList);

    SERIALIZE_VAR(LoginServerPort);
    SERIALIZE_VAR(AuthServerPort);
    SERIALIZE_VAR(GameServerPort);
    SERIALIZE_VAR(WebUIServerPort);
    SERIALIZE_VAR(WebUIServerUsername);
    SERIALIZE_VAR(WebUIServerPassword);
    SERIALIZE_VAR(Announcements);
    SERIALIZE_VAR(DatabaseTrimInterval);
    SERIALIZE_VAR(BloodMessageMaxLivePoolEntriesPerArea);
    SERIALIZE_VAR(BloodMessageMaxDatabaseEntries);
    SERIALIZE_VAR(BloodMessagePrimeCountPerArea);
    SERIALIZE_VAR(BloodstainMaxLivePoolEntriesPerArea);
    SERIALIZE_VAR(BloodstainMaxDatabaseEntries);
    SERIALIZE_VAR(BloodstainPrimeCountPerArea);
    SERIALIZE_VAR(GhostMaxLivePoolEntriesPerArea);
    SERIALIZE_VAR(GhostPrimeCountPerArea);
    SERIALIZE_VAR(GhostPrimeCountPerArea);
    SERIALIZE_VAR(GhostMaxDatabaseEntries);
    SERIALIZE_VAR(QuickMatchWinXp);
    SERIALIZE_VAR(QuickMatchLoseXp);
    SERIALIZE_VAR(QuickMatchDrawXp);
    SERIALIZE_VAR(QuickMatchRankXp);
    SERIALIZE_VAR(DisableInvasions);
    SERIALIZE_VAR(DisableCoop);
    SERIALIZE_VAR(DisableBloodMessages);
    SERIALIZE_VAR(DisableBloodStains);
    SERIALIZE_VAR(DisableGhosts);
    SERIALIZE_VAR(DisableInvasionAutoSummon);
    SERIALIZE_VAR(DisableCoopAutoSummon);
    SERIALIZE_VAR(IgnoreInvasionAreaFilter)
    SERIALIZE_STRUCT_VAR(SummonSignMatchingParameters);
    SERIALIZE_STRUCT_VAR(WayOfBlueMatchingParameters);
    SERIALIZE_STRUCT_VAR(DarkSpiritInvasionMatchingParameters);
    SERIALIZE_STRUCT_VAR(MoundMakerInvasionMatchingParameters);
    SERIALIZE_STRUCT_VAR(CovenantInvasionMatchingParameters);
    SERIALIZE_STRUCT_VAR(UndeadMatchMatchingParameters);

    return true;
}

#undef SERIALIZE_STRUCT_VAR
#undef SERIALIZE_VAR