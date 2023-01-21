/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/WinApi.h"

#ifdef _WIN32

#include <windows.h>
#include <TlHelp32.h>

std::pair<intptr_t, size_t> GetModuleBaseRegion(const char* ModuleName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 moduleEntry = {};
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(snapshot, &moduleEntry))
        {
            do 
            {
                if (_stricmp(moduleEntry.szModule, ModuleName) == 0)
                {
                    return { reinterpret_cast<intptr_t>(moduleEntry.modBaseAddr), moduleEntry.modBaseSize };
                    
                }
            }
            while (Module32Next(snapshot, &moduleEntry));
        }

        CloseHandle(snapshot);
    }

    return { 0, 0 };
}

#endif