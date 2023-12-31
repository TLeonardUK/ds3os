/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/Rtti.h"

#include <vector>
#include <string>

// We only support resolving RTTI information on msvc.
// Adding support should be easy enough for other compilers if required.
#ifdef _MSC_VER

#include <ehdata.h>
#include <rttidata.h>

std::string DemangleTypeName(const char* Mangled)
{
    // Turbo garbage demangler, basically just strips off all non-identifier characters.
    // Works fine for our purposes though.

    size_t MangledLength = strlen(Mangled);
    std::string Unmangled = "";

    size_t Index = 0;

    // Read prefix.
    if (Index < MangledLength && Mangled[Index] == '.')
    {
        Index++;
    }
    if (Index < MangledLength && Mangled[Index] == '?')
    {
        Index++;
    }

    // Read complex type information.
    if (Index < MangledLength && Mangled[Index] == 'A')
    {
        Index++;
    }
    if (Index < MangledLength && Mangled[Index] == 'V')
    {
        Index++;
    }

    // Read the rest of the name.
    std::vector<std::string> Scopes;

    for (/*Empty*/; Index < MangledLength; /* empty */)
    {
        char Chr = Mangled[Index];
        switch (Chr)
        {
        case '@':
            {
                Scopes.push_back(Unmangled);
                Unmangled = "";

                // Two @@'s represent end of type name.
                if (Index < MangledLength - 1 && Mangled[Index + 1] == '@')
                {
                    Index += 2;
                    break;
                }

                Index++;
                break;
            }
        default:
            {
                Unmangled += Chr;
                Index++;
                break;
            }
        }
    }

    // Generate final unmangled name.
    Unmangled = "";
    for (size_t i = 0; i < Scopes.size(); i++)
    {
        if (i > 0)
        {
            Unmangled += "::";
        }
        Unmangled += Scopes[Scopes.size() - (i + 1)];
    }

    return Unmangled;
}

std::string GetRttiNameFromObject(void* ptr)
{
    _RTTICompleteObjectLocator* Locator = reinterpret_cast<_RTTICompleteObjectLocator***>(ptr)[0][-1];
    uintptr_t ImageBase = reinterpret_cast<uintptr_t>(Locator) - Locator->pSelf;
    TypeDescriptor* TypeDescriptorInstance = reinterpret_cast<TypeDescriptor*>(ImageBase + Locator->pTypeDescriptor);
    return DemangleTypeName(TypeDescriptorInstance->name);
}

#else

std::string GetRttiNameFromObject(void* ptr)
{
    return "";
}

#endif