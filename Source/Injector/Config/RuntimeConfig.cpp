/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Config/RuntimeConfig.h"
#include "Shared/Core/Utils/File.h"

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

#define SERIALIZE_VAR(x) SerializeVar(Json, #x, x, Loading);
#define SERIALIZE_STRUCT_VAR(x) SerializeStructVar(Json, #x, x, Loading);

bool RuntimeConfig::Serialize(nlohmann::json& Json, bool Loading)
{
    SERIALIZE_VAR(ServerName);
    SERIALIZE_VAR(ServerHostname);
    SERIALIZE_VAR(ServerPublicKey);
    SERIALIZE_VAR(EnableSeperateSaveFiles);
    SERIALIZE_VAR(ServerPort);
    SERIALIZE_VAR(ServerGameType);

    return true;
}

#undef SERIALIZE_STRUCT_VAR
#undef SERIALIZE_VAR