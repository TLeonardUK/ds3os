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
#include "Core/Utils/Logging.h"
#include "Platform/Platform.h"

#include <filesystem>

#include <steam/steam_api.h>

extern "C" void __cdecl SteamWarningHook(int nSeverity, const char* pchDebugText)
{
    Log("[Steam] %i: %s", nSeverity, pchDebugText);
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

    Log("Dark Souls 3 - Open Server");
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
            Error("Failed to initialize steam api.");
            return 1;
        }
        SteamUtils()->SetWarningMessageHook(&SteamWarningHook);
    }

    // TODO: Split this out into a seperate application.
    // TODO: Also do less crappy arg parsing.
    if (start_as_client_emulator)
    {
        Client ClientInstance;
        if (!ClientInstance.Init())
        {
            Error("Client emulator failed to initialize.");
            return 1;
        }
        ClientInstance.RunUntilQuit();
        if (!ClientInstance.Term())
        {
            Error("Client emulator failed to terminate.");
            return 1;
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

    if (!PlatformTerm())
    {
        Error("Failed to tidy up platform specific functionality.");
        return 1;
    }

    return 0;
}