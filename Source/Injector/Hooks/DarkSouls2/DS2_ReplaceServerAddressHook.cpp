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

bool DS2_ReplaceServerAddressHook::Install(Injector& injector)
{   
    // TODO
    return true;
}

void DS2_ReplaceServerAddressHook::Uninstall()
{

}

const char* DS2_ReplaceServerAddressHook::GetName()
{
    return "DS2 Replace Server Address";
}

