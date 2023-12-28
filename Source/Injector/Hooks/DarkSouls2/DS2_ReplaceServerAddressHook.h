/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Injector/Hooks/Hook.h"

// Hooks part of the STL to monitor for the server address and replace it when
// its found. Adding it to the STL adds some constant overhead but should be reliable
// for all game versions as its non-obfuscated and unlikely to ever be obfuscated due to 
// performance concerns.
class DS2_ReplaceServerAddressHook : public Hook
{
public:
    virtual bool Install(Injector& injector) override;
    virtual void Uninstall() override;
    virtual const char* GetName() override;

private:
    bool PatchKey(Injector& injector);
    bool PatchHostname(Injector& injector);

};
