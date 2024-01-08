/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Hooks/DarkSouls2/DS2_ReplaceServerAddressHook.h"
#include "Injector/Injector/Injector.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "ThirdParty/detours/src/detours.h"

#include <vector>
#include <iterator>

// This is all much simpler than DS3, the key isn't encrypted or obfuscated, so we can just
// straight patch it in memory.

bool DS2_ReplaceServerAddressHook::Install(Injector& injector)
{   
    return PatchKey(injector) && PatchHostname(injector);
}

bool DS2_ReplaceServerAddressHook::PatchHostname(Injector& injector)
{
    const RuntimeConfig& Config = Injector::Instance().GetConfig();
    std::wstring WideHostname = WidenString(Config.ServerHostname);
    size_t CopyLength = (WideHostname.size() + 1) * 2;

    while (true)
    {
        std::vector<intptr_t> address_matches = injector.SearchString({
            L"frpg2-steam64-ope-login.fromsoftware-game.net"
        });

        bool FoundKey = false;

        for (intptr_t key : address_matches)
        {
            // If the memory is not writable yet, modify its protection (steam drm fucks with the protection during boot).
            MEMORY_BASIC_INFORMATION info;
            if (VirtualQuery((void*)key, &info, sizeof(info)) == 0)
            {
                continue;
            }
            if (((info.Protect & PAGE_READWRITE) == 0 && (info.Protect & PAGE_EXECUTE_READWRITE) == 0))
            {
                continue;
            }

            wchar_t* ptr = (wchar_t*)key;
            for (size_t i = 0; i < WideHostname.size() + 1; i++)
            {
                wchar_t chr = WideHostname[i];

                // Flip endian
                char* source = (char*)&chr;
                std::swap(source[0], source[1]);

                memcpy(ptr, source, sizeof(wchar_t));
                ptr++;
            }
            FoundKey = true;
        }

        if (FoundKey)
        {
            break;
        }
    }

    return true;
}

bool DS2_ReplaceServerAddressHook::PatchKey(Injector& injector)
{
    while (true)
    {
        const RuntimeConfig& Config = Injector::Instance().GetConfig();
        size_t CopyLength = Config.ServerPublicKey.size() + 1;

        std::vector<intptr_t> key_matches = injector.SearchString({
            "-----BEGIN RSA PUBLIC KEY-----\n"
            "MIIBCAKCAQEAxSeDuBTm3AytrIOGjDKpwJY+437i1F8leMBASVkknYdzM5HB4z8X\n"
            "YTXDylr/N6XAhgr/LcFFZ68yQNQ4AquriMONB+TWUiX0xu84ixYH3AqRtIVqLQbQ\n"
            "xKZsTfyCRC94n9EnvPeS+ueM495YhLIJQBf9T2aCeoHZBFDh2CghJQCdyd4dOT/E\n"
            "9ZxPImwj1t2fZkkKo4smpGk7GcCask2SGsnk/P2jUJxsOyFlCojaW1IldPxn+lXH\n"
            "dlgHSLjQvMlWiZ2SmOwvJqPWMv6XyUXYqsOdejRJJQjV7jeDzYG8trX+bSQxnTAw\n"
            "ENjvjslEcjBmzOCiqFTA/9H1jMjReZpI/wIBAw==\n"
            "-----END RSA PUBLIC KEY-----\n"
        });

        bool FoundKey = false;

        for (intptr_t key : key_matches)
        {
            // If the memory is not writable yet, modify its protection (steam drm fucks with the protection during boot).
            MEMORY_BASIC_INFORMATION info;
            if (VirtualQuery((void*)key, &info, sizeof(info)) == 0 ||
                ((info.Protect & PAGE_READWRITE) == 0 && (info.Protect & PAGE_EXECUTE_READWRITE) == 0))
            {
                continue;
            }

            memcpy((char*)key, Config.ServerPublicKey.c_str(), CopyLength);
            FoundKey = true;
        }

        if (FoundKey)
        {
            break;
        }
    }

    return true;
}

void DS2_ReplaceServerAddressHook::Uninstall()
{

}

const char* DS2_ReplaceServerAddressHook::GetName()
{
    return "DS2 Replace Server Address";
}

