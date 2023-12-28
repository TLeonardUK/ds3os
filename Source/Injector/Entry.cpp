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

#include "Injector/Injector.h"

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
    // Alloc a new console and redirect standard IO to it.
    FILE* dummy;
    AllocConsole();
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONOUT$", "w", stdout);

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

    if (!PlatformInit())
    {
        Error("Failed to initialize platform specific functionality.");
        return;
    }

    Injector InjectorInstance;
    if (!InjectorInstance.Init())
    {
        Error("Injector failed to initialize.");
        MessageBoxA(nullptr, "Failed to initialize DS3OS.\n\nGame will now be terminated to avoid playing on a partially patched game which may trigger bans.", "DS3OS Error", MB_OK|MB_ICONEXCLAMATION);
        std::abort();
        return;
    }
    InjectorInstance.RunUntilQuit();
    if (!InjectorInstance.Term())
    {
        Error("Injector failed to terminate.");
        return;
    }

    if (!PlatformTerm())
    {
        Error("Failed to tidy up platform specific functionality.");
        return;
    }
}