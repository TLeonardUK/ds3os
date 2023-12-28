/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Hooks/DarkSouls3/DS3_ReplaceServerAddressHook.h"
#include "Injector/Injector/Injector.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "ThirdParty/detours/src/detours.h"

#include <vector>
#include <iterator>

namespace 
{
    using vector_insert_range_p = void(__fastcall*)(void* self, void* where, void* first, void* last);
    vector_insert_range_p s_original_vector_insert_range;

    void __fastcall VectorInsertRangeHook(void* self, void* where, void* first, void* last)
    {    
        const char* k_retail_key = "-----BEGIN RSA PUBLIC KEY-----\nMIIBCgKCAQEA1Nuliw8Rvkt40+0OKoW0JpuSIU/ErQwjzRicZV9JDrCikiTIqoAh\nvBj3DcHwGX1d6T5PY27E4SHa24eRxDetMPEYKeclUeJ0jB07lCtH9Y0zMWl1PMfo\nlIgcm5VKfz+Ua+Ny6klgx1y3ODxMS9g0k11t1WsFtccr464lfP4i1Fgz1/C2Jmgu\n7EV+YdIYkOqT+NJtJG5Z75guq/rTQ85/tVuBKa9dvGIaAqG+nTVlJ2+vzKhjPVXJ\n6AwzWdAbG802uzNC9pk+LEQ+YZXCZSHPMNKz6IwXjlagqDxl2w0rg6dEEFxRY0lm\nS0nqh01eO9pYZA2k0TmpeWHhrKJrnvrKFwIDAQAB\n-----END RSA PUBLIC KEY-----\n";

        const size_t k_key_length = 426;
        const size_t k_block_length = 520;
        const size_t k_hostname_offset = 432;

        char* first_c = reinterpret_cast<char*>(first);
        char* last_c = reinterpret_cast<char*>(last);

        size_t distance = std::distance(first_c, last_c);
        if (distance == k_key_length)
        {
            std::string str(first_c, distance);
            if (str.find(k_retail_key) == 0)
            {
                Log("Retail server address requested, patching to custom server address.");

                const RuntimeConfig& Config = Injector::Instance().GetConfig();

                std::wstring UnicodeHostname = WidenString(Config.ServerHostname);

                memset(first_c, 0, k_block_length);
                memcpy(first_c, Config.ServerPublicKey.c_str(), Config.ServerPublicKey.size() + 1); // +1 as we want the null terminator.
                memcpy(first_c + k_hostname_offset, UnicodeHostname.c_str(), UnicodeHostname.size() * sizeof(wchar_t) + sizeof(wchar_t));  // +1 as we want the null terminator.
            }
        }

        s_original_vector_insert_range(self, where, first, last);
    }
};

bool DS3_ReplaceServerAddressHook::Install(Injector& injector)
{
    // This matches std::vector::insert_range.
    std::vector<intptr_t> matches = injector.SearchAOB({ 
        0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x53, 0x56, 0x57, 0x41, 0x54, 
        0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xec, 0x50, 0x48, 0xc7, 0x44, 0x24, 0x30, 
        0xfe, 0xff, 0xff, 0xff, 0x4d, 0x8b, 0xf9, 0x4d, 0x8b, 0xe0, 0x48, 0x8b, 0xd9, 0x49, 0x8b, 
        0xf9, 0x49, 0x2b, 0xf8, 0x0f, 0x84, 0x52, 0x01, 0x00, 0x00, 0x48, 0x8b, 0x71, 0x10, 0x4c, 
        0x8b, 0x41, 0x08, 0x48, 0x8b, 0xc6, 0x49, 0x2b, 0xc0, 0x48, 0x3b, 0xc7
    });

    if (matches.size() == 0)
    {
        Error("Failed to find injection point for modifying server address.");
        return false;
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    for (intptr_t func_ptr : matches)
    {
        // We end up detouring all the functions back to a single instance of the function.
        // As the functions are identical though, this makes no practical difference.
        s_original_vector_insert_range = reinterpret_cast<vector_insert_range_p>(func_ptr);
        DetourAttach(&(PVOID&)s_original_vector_insert_range, VectorInsertRangeHook);
    }
    DetourTransactionCommit();

    return true;
}

void DS3_ReplaceServerAddressHook::Uninstall()
{

}

const char* DS3_ReplaceServerAddressHook::GetName()
{
    return "DS3 Replace Server Address";
}

