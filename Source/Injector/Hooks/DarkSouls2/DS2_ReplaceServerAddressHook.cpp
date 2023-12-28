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
    std::vector<intptr_t> address_matches = injector.SearchString({
        L"frpg2-steam64-ope-login.fromsoftware-game.net"
    });

    if (address_matches.size() != 1)
    {
        Error("Expected to find one instance of server address, but found %zi.", address_matches.size());
        return false;
    }

    const RuntimeConfig& Config = Injector::Instance().GetConfig();
    std::wstring WideHostname = WidenString(Config.ServerHostname);
    memcpy((char*)address_matches[0], WideHostname.c_str(), (WideHostname.size() + 1) * 2);

    return true;
}

bool DS2_ReplaceServerAddressHook::PatchKey(Injector& injector)
{
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

    if (key_matches.size() != 1)
    {
        Error("Expected to find one instance of server key, but found %zi.", key_matches.size());
        return false;
    }

    const RuntimeConfig& Config = Injector::Instance().GetConfig();
    memcpy((char*)key_matches[0], Config.ServerPublicKey.c_str(), Config.ServerPublicKey.size() + 1);

    return true;
}

void DS2_ReplaceServerAddressHook::Uninstall()
{

}

const char* DS2_ReplaceServerAddressHook::GetName()
{
    return "DS2 Replace Server Address";
}

