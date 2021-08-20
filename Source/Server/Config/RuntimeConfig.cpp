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
    catch (nlohmann::json::parse_error& ex)
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

bool RuntimeConfigAnnouncement::Serialize(nlohmann::json& Json, bool Loading)
{
    SerializeVar(Json, "Header",              Header,                 Loading);
    SerializeVar(Json, "Body",                Body,                   Loading);

    return true;
}

bool RuntimeConfig::Serialize(nlohmann::json& Json, bool Loading)
{
    SerializeVar(Json, "ServerName",                                    ServerName,                                     Loading);
    SerializeVar(Json, "ServerDescription",                             ServerDescription,                              Loading);
    SerializeVar(Json, "ServerHostname",                                ServerHostname,                                 Loading);
    SerializeVar(Json, "ServerIP",                                      ServerIP,                                       Loading);
    SerializeVar(Json, "LoginServerPort",                               LoginServerPort,                                Loading);
    SerializeVar(Json, "AuthServerPort",                                AuthServerPort,                                 Loading);
    SerializeVar(Json, "GameServerPort",                                GameServerPort,                                 Loading);
    SerializeVar(Json, "Announcements",                                 Announcements,                                  Loading);
    SerializeVar(Json, "BloodMessageMaxLivePoolEntriesPerArea",         BloodMessageMaxLivePoolEntriesPerArea,          Loading);
    SerializeVar(Json, "BloodMessagePrimeCountPerArea",                 BloodMessagePrimeCountPerArea,                  Loading);

    return true;
}
