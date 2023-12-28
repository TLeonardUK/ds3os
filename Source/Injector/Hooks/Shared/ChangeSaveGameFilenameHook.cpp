/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Hooks/Shared/ChangeSaveGameFilenameHook.h"
#include "Injector/Injector/Injector.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "ThirdParty/detours/src/detours.h"

#include <vector>
#include <iterator>
#include <Windows.h>

namespace 
{
    using create_file_p = HANDLE(WINAPI*)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    create_file_p s_original_create_file = CreateFileW;

    HANDLE WINAPI CreateFileHook(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
    {
        std::string filename = NarrowString(lpFileName);

        const std::string extension_sl2 = ".sl2";
        const std::string extension_sl2_bak = ".sl2.bak";
        const std::string extension_ds3os = ".ds3os";

        if (StringEndsWith(filename, extension_sl2))
        {
            filename = filename.substr(0, filename.size() - extension_sl2.size()) + extension_ds3os;
        }
        else if (StringEndsWith(filename, extension_sl2_bak))
        {
            filename = filename.substr(0, filename.size() - extension_sl2_bak.size()) + extension_ds3os;
        }

        std::wstring new_filename = WidenString(filename);
        return s_original_create_file(new_filename.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
};

bool ChangeSaveGameFilenameHook::Install(Injector& injector)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)s_original_create_file, CreateFileHook);
    DetourTransactionCommit();

    return true;
}

void ChangeSaveGameFilenameHook::Uninstall()
{

}

const char* ChangeSaveGameFilenameHook::GetName()
{
    return "Change Save Game Filename";
}

