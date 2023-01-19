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

#include <thread>
#include <chrono>
#include <fstream>

#include "ThirdParty/nlohmann/json.hpp"

namespace 
{
    void dummyFunction()
    {
    }
};

Injector::Injector()
{
}

Injector::~Injector()
{
}

bool Injector::Init()
{
    Log("Initializing injector ...");

    // Grab the dll path based on the location of static function.
    HMODULE moduleHandle = nullptr;
    char modulePath[MAX_PATH];
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummyFunction, &moduleHandle) == 0)
    {
        Error("Failed to get dll handle, GetLastError=%u", GetLastError());
        return false;
    }
    if (GetModuleFileName(moduleHandle, modulePath, sizeof(modulePath)) == 0)
    {
        Error("Failed to get dll path, GetLastError=%u", GetLastError());
        return false;
    }

    DllPath = modulePath;
    DllPath = DllPath.parent_path();

    Log("DLL Path: %s", DllPath.string().c_str());

    // TODO: Move this stuff into a RuntimeConfig type class.
    ConfigPath = DllPath / std::filesystem::path("Injector.config");

    // Load configuration.
    if (!Config.Load(ConfigPath))
    {
        Error("Failed to load configuration file: %s", ConfigPath.string().c_str());
        return false;
    }


    Log("Server Name: %s", Config.ServerName.c_str());
    Log("Server Hostname: %s", Config.ServerHostname.c_str());

    return true;
}

bool Injector::Term()
{
    Log("Terminating injector ...");

    return true;
}

void Injector::RunUntilQuit()
{
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
