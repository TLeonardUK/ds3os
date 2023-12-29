/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Injector.h"
#include "Injector/Config/BuildConfig.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/Random.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "Shared/Core/Network/NetUtils.h"
#include "Shared/Core/Network/NetHttpRequest.h"
#include "Shared/Core/Utils/WinApi.h"
#include "Shared/Core/Utils/Strings.h"

#include "Injector/Hooks/DarkSouls3/DS3_ReplaceServerAddressHook.h"
#include "Injector/Hooks/DarkSouls2/DS2_ReplaceServerAddressHook.h"
#include "Injector/Hooks/Shared/ReplaceServerPortHook.h"
#include "Injector/Hooks/Shared/ChangeSaveGameFilenameHook.h"

#include <thread>
#include <chrono>
#include <fstream>

#include "ThirdParty/nlohmann/json.hpp"

#include <Windows.h>

// Use different save file.
// add checkbox to ui to show debug window.

namespace 
{
    void dummyFunction()
    {
    }
};

Injector& Injector::Instance()
{
    return *s_instance;
}

Injector::Injector()
{
    s_instance = this;
}

Injector::~Injector()
{
    s_instance = nullptr;
}

bool Injector::Init()
{
    Log("Initializing injector ...");

    // Grab the dll path based on the location of static function.
    HMODULE moduleHandle = nullptr;
    wchar_t modulePath[MAX_PATH] = {};
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummyFunction, &moduleHandle) == 0)
    {
        Error("Failed to get dll handle, GetLastError=%u", GetLastError());
        return false;
    }
    if (GetModuleFileNameW(moduleHandle, modulePath, sizeof(modulePath)) == 0)
    {
        Error("Failed to get dll path, GetLastError=%u", GetLastError());
        return false;
    }

    std::wstring modulePathWide = modulePath;
    DllPath = modulePathWide;
    DllPath = DllPath.parent_path();

    Log("DLL Path: %s", NarrowString(DllPath.generic_wstring()).c_str());

    // TODO: Move this stuff into a RuntimeConfig type class.
    ConfigPath = DllPath / std::filesystem::path("Injector.config");

    // Load configuration.
    if (!Config.Load(ConfigPath))
    {
        Error("Failed to load configuration file: %s", ConfigPath.string().c_str());
        return false;
    }

    if (!ParseGameType(Config.ServerGameType.c_str(), CurrentGameType))
    {
        Error("Unknown game type in configuration file: %s", Config.ServerGameType.c_str());
        return false;
    }

    Log("Server Name: %s", Config.ServerName.c_str());
    Log("Server Hostname: %s", Config.ServerHostname.c_str());
    Log("Server Port: %i", Config.ServerPort);
    Log("Server Game Type: %s", Config.ServerGameType.c_str());

    switch (CurrentGameType)
    {
        case GameType::DarkSouls3:
        {
            ModuleRegion = GetModuleBaseRegion("DarkSoulsIII.exe");
            break;
        }
        case GameType::DarkSouls2:
        {
            ModuleRegion = GetModuleBaseRegion("DarkSoulsII.exe");
            break;
        }
    }

    Log("Base Address: 0x%p",ModuleRegion.first);
    Log("Base Size: 0x%08x", ModuleRegion.second);
    Log("");

    if (ModuleRegion.first == 0)
    {
        Error("Failed to get module region for DarkSoulsIII.exe.");
        return false;
    }

    // Add hooks we need to use based on configuration.
    switch (CurrentGameType)
    {
        case GameType::DarkSouls3:
        {
            Hooks.push_back(std::make_unique<DS3_ReplaceServerAddressHook>());
            break;
        }
        case GameType::DarkSouls2:
        {
            Hooks.push_back(std::make_unique<DS2_ReplaceServerAddressHook>());
            break;
        }
    }

    Hooks.push_back(std::make_unique<ReplaceServerPortHook>());
    if (Config.EnableSeperateSaveFiles)
    {
        Hooks.push_back(std::make_unique<ChangeSaveGameFilenameHook>());
    }

    Log("Installing hooks ...");
    bool AllInstalled = true;
    for (auto& hook : Hooks)
    {
        if (hook->Install(*this))
        {
            Success("\t%s: Success", hook->GetName());
            InstalledHooks.push_back(hook.get());
        }
        else
        {
            Error("\t%s: Failed", hook->GetName());
            AllInstalled = false;
        }
    }

    if (!AllInstalled)
    {
        return false;
    }

    return true;
}

bool Injector::Term()
{
    Log("Uninstalling hooks ...");
    for (auto& hook : InstalledHooks)
    {
        Error("\t%s", hook->GetName());
    }
    InstalledHooks.clear();

    Log("Terminating injector ...");

    return true;
}

void Injector::RunUntilQuit()
{
    Log("");
    Success("Injector is now running.");

    // We should really do this event driven ...
    // This suffices for now.
    while (!QuitRecieved)
    {
        // TODO: Do any polling we need to do here ...

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Injector::SaveConfig()
{
    if (!Config.Save(ConfigPath))
    {
        Error("Failed to save configuration file: %s", ConfigPath.string().c_str());
    }
}

intptr_t Injector::GetBaseAddress()
{
    return ModuleRegion.first;
}

std::vector<intptr_t> Injector::SearchAOB(const std::vector<AOBByte>& pattern)
{
    std::vector<intptr_t> Matches;

    size_t ScanLength = ModuleRegion.second - pattern.size();

    for (size_t Offset = 0; Offset < ScanLength; Offset++)
    {
        bool OffsetMatches = true;

        for (size_t PatternOffset = 0; PatternOffset < pattern.size(); PatternOffset++)
        {
            if (pattern[PatternOffset].has_value())
            {
                intptr_t Address = ModuleRegion.first + Offset + PatternOffset;
                uint8_t Byte = *reinterpret_cast<uint8_t*>(Address);
                if (Byte != pattern[PatternOffset].value())
                {
                    OffsetMatches = false;
                    break;
                }
            }
        }        

        if (OffsetMatches)
        {
            Matches.push_back(ModuleRegion.first + Offset);
        }
    }

    return Matches;
}


std::vector<intptr_t> Injector::SearchString(const std::string& input)
{
    std::vector<AOBByte> aob;
    aob.reserve(input.size());

    for (size_t i = 0; i < input.size(); i++)
    {
        aob.push_back(input[i]);
    }

    return SearchAOB(aob);
}

std::vector<intptr_t> Injector::SearchString(const std::wstring& input)
{
    std::vector<AOBByte> aob;
    aob.reserve(input.size() * 2);

    for (size_t i = 0; i < input.size(); i++)
    {
        wchar_t val = input[i];
        uint8_t upper = (val >> 8) & 0xFF;
        uint8_t lower = (val >> 0) & 0xFF;
        aob.push_back(upper);
        aob.push_back(lower);
    }

    return SearchAOB(aob);
}