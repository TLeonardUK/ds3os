/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

class Injector;

// Base class for all detour hooks.
class Hook
{
public:

    // Installs the hook, returns true on success.
    virtual bool Install(Injector& injector) = 0;

    // Uninstalls the hook.
    virtual void Uninstall() = 0;

    // Gets a descriptive name for what this hook is doing.
    virtual const char* GetName() = 0;

};
