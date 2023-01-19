/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Shared/Platform/Platform.h"
#include "Config/RuntimeConfig.h"

#include <memory>
#include <vector>
#include <filesystem>
#include <queue>
#include <unordered_map>

// Core of this application, manages all the 
// network services that ds3 uses. 

class Injector
{
public:
    Injector();
    ~Injector();

    bool Init();
    bool Term();
    void RunUntilQuit();

    void SaveConfig();

    const RuntimeConfig& GetConfig()    { return Config; }

private:

    bool QuitRecieved = false;

    std::filesystem::path DllPath;
    std::filesystem::path ConfigPath;

    RuntimeConfig Config;

};