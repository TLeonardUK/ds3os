/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Server.h"
#include "Client/Client.h"
#include "Config/BuildConfig.h"
#include "Core/Utils/Logging.h"
#include "Platform/Platform.h"

#include <filesystem>
#include <thread>

#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

extern "C" void __cdecl SteamWarningHook(int nSeverity, const char* pchDebugText)
{
    LogS("Steam", "%i: %s", nSeverity, pchDebugText);
}

int main(int argc, char* argv[])
{
    bool start_as_client_emulator = false;
    std::string mode_arg = argc > 1 ? argv[1] : "";
    start_as_client_emulator = (mode_arg == "-client_emulator");

    // Switch working directory to the same directory the
    // exe is inside of. Prevents wierdness when we start from visual studio etc.
    std::filesystem::path exe_directory = std::filesystem::path(argv[0]).parent_path();
    std::filesystem::current_path(exe_directory);

    Log(R"--(    ____             __      _____             __        _____)--");
    Log(R"--(   / __ \____ ______/ /__   / ___/____  __  __/ /____   |__  /)--");
    Log(R"--(  / / / / __ `/ ___/ //_/   \__ \/ __ \/ / / / / ___/    /_ < )--");
    Log(R"--( / /_/ / /_/ / /  / ,<     ___/ / /_/ / /_/ / (__  )   ___/ / )--");
    Log(R"--(/_____/\__,_/_/  /_/|_|   /____/\____/\__,_/_/____/   /____/  )--");
    Log(R"--(  / __ \____  ___  ____     / ___/___  ______   _____  _____  )--");
    Log(R"--( / / / / __ \/ _ \/ __ \    \__ \/ _ \/ ___/ | / / _ \/ ___/  )--");
    Log(R"--(/ /_/ / /_/ /  __/ / / /   ___/ /  __/ /   | |/ /  __/ /      )--");
    Log(R"--(\____/ .___/\___/_/ /_/   /____/\___/_/    |___/\___/_/       )--");
    Log(R"--(    /_/                                                       )--");
    Log("");
    Log("https://github.com/tleonarduk/ds3os");
    Log("");

    if (!PlatformInit())
    {
        Error("Failed to initialize platform specific functionality.");
        return 1;
    }

    if (start_as_client_emulator)
    {
        if (!SteamAPI_Init())
        {
            Error("Failed to initialize steam api, please ensure steam is running.");
            return 1;
        }
        SteamUtils()->SetWarningMessageHook(&SteamWarningHook);

    }
    else
    {
        // Ports etc are irrelevant, we're only using the api to do authentication. 
        if (!SteamGameServer_Init(0, 50001, 50002, eServerModeAuthentication, "1.0.0.0"))
        {
            Error("Failed to initialize steam game server api.");
            return 1;
        }
        SteamGameServerUtils()->SetWarningMessageHook(&SteamWarningHook);
    }

    // TODO: Split this out into a seperate application.
    // TODO: Also do less crappy arg parsing.
    if (start_as_client_emulator)
    {
        std::array<std::thread, BuildConfig::CLIENT_EMULATOR_COUNT> ClientThreads;

        for (size_t i = 0; i < BuildConfig::CLIENT_EMULATOR_COUNT; i++)
        {
            ClientThreads[i] = std::thread([i]() {

                Client ClientInstance;

                if (!ClientInstance.Init(true, i))
                {
                    Error("Client emulator failed to initialize.");
                    return;
                }

                ClientInstance.RunUntilQuit();
                
                if (!ClientInstance.Term())
                {
                    Error("Client emulator failed to terminate.");
                    return;
                }

            });            
        }

        for (size_t i = 0; i < BuildConfig::CLIENT_EMULATOR_COUNT; i++)
        {
            ClientThreads[i].join();
        }
    }
    else
    {
        Server ServerInstance; 
        if (!ServerInstance.Init())
        {
            Error("Server failed to initialize.");
            return 1;
        }
        ServerInstance.RunUntilQuit();
        if (!ServerInstance.Term())
        {
            Error("Server failed to terminate.");
            return 1;
        }
    }

    if (start_as_client_emulator)
    {
        SteamAPI_Shutdown();
    }
    else
    {
        SteamGameServer_Shutdown();
    }

    if (!PlatformTerm())
    {
        Error("Failed to tidy up platform specific functionality.");
        return 1;
    }

    return 0;
}