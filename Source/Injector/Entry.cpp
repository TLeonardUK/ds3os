/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license. 
 * You should have received a copy of the license along with this program. 
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include <windows.h>
#include <filesystem>

#include "Shared/Core/Utils/Logging.h"

void main();

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, NULL, 0, NULL);
        break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

void main()
{
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
        return;
    }

    // Load config information.

    // Hook stuff

    // Wait until terminating.

    if (!PlatformTerm())
    {
        Error("Failed to tidy up platform specific functionality.");
        return;
    }
}