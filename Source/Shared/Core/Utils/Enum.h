/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "Shared/Core/Utils/Logging.h"

template <typename T>
inline std::string GetEnumString(T Input)
{
    return "";
}

template <typename T>
inline T GetEnumValue(const std::string& Input)
{
    Ensure(false);
    return T();
}

template <typename T>
inline const std::vector<T>* GetEnumValues()
{
    Ensure(false);
    return nullptr;
}