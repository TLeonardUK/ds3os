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

int main(int argc, char* argv[])
{
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

    // TODO: Split this out into a seperate application.
    // TODO: Also do less crappy arg parsing.
    std::string mode_arg = argc > 1 ? argv[1] : "";
    if (mode_arg == "-client_emulator")
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
    
    if (!PlatformTerm())
    {
        Error("Failed to tidy up platform specific functionality.");
        return 1;
    }

    return 0;
}