/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/ServerManager.h"
#include "Client/Client.h"
#include "Config/BuildConfig.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Platform/Platform.h"

// DEBUG DEBUG DEBUG
#include "Shared/Core/Utils/Protobuf.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
//#define DEBUG_TEST
// DEBUG DEBUG DEBUG

#include <filesystem>
#include <thread>

#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

extern "C" void __cdecl SteamWarningHook(int nSeverity, const char* pchDebugText)
{
    LogS("Steam", "%i: %s", nSeverity, pchDebugText);
}

#ifdef DEBUG_TEST
#pragma optimize("", off)
void DebugTest()
{
    DecodedProtobufRegistry registry;

    for (const auto& entry : std::filesystem::directory_iterator("Z:/ds3os/Temp/ProtobufDump"))
    {
        if (entry.is_regular_file())
        {
            std::filesystem::path path = entry.path();

            std::vector<uint8_t> bytes;
            if (!ReadBytesFromFile(path, bytes))
            {
                break;
            }

            std::string className = path.stem().string().c_str();
            if (size_t pos = className.find("_"); pos != std::string::npos)
            {
                className = className.substr(pos + 1);
            }

            registry.Decode(className, bytes.data(), bytes.size());
        }        
    }

    std::string result = registry.ToString();
    Log("%s", result.c_str());

    Log("Loaded protobuf registry.");
}
#endif

int main(int argc, char* argv[])
{
    bool start_as_client_emulator = false;
    std::string mode_arg = argc > 1 ? argv[1] : "";
    start_as_client_emulator = (mode_arg == "-client_emulator");

    // Switch working directory to the same directory the
    // exe is inside of. Prevents wierdness when we start from visual studio etc.
    std::filesystem::path exe_directory = std::filesystem::path(argv[0]).parent_path();
    std::filesystem::current_path(exe_directory);

    Log(R"--(    ____             __      _____             __    )--");
    Log(R"--(   / __ \____ ______/ /__   / ___/____  __  __/ /____)--");
    Log(R"--(  / / / / __ `/ ___/ //_/   \__ \/ __ \/ / / / / ___/)--");
    Log(R"--( / /_/ / /_/ / /  / ,<     ___/ / /_/ / /_/ / (__  ) )--");
    Log(R"--(/_____/\__,_/_/  /_/|_|   /____/\____/\__,_/_/____/  )--");
    Log(R"--(  / __ \____  ___  ____     / ___/___  ______   _____  _____  )--");
    Log(R"--( / / / / __ \/ _ \/ __ \    \__ \/ _ \/ ___/ | / / _ \/ ___/  )--");
    Log(R"--(/ /_/ / /_/ /  __/ / / /   ___/ /  __/ /   | |/ /  __/ /      )--");
    Log(R"--(\____/ .___/\___/_/ /_/   /____/\___/_/    |___/\___/_/       )--");
    Log(R"--(    /_/                                                       )--");
    Log("");
    Log("https://github.com/tleonarduk/ds3os");
    Log("");

#ifdef DEBUG_TEST
    DebugTest();
#endif

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
        ServerManager ServerManagerInstance;
        if (!ServerManagerInstance.Init())
        {
            Error("Server failed to initialize.");
            return 1;
        }
        ServerManagerInstance.RunUntilQuit();
        if (!ServerManagerInstance.Term())
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