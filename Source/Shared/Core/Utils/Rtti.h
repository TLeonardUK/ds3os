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

// Takes an arbitrary pointer to a polymorphic object and attempts
// to get its RTTI demangled name.
std::string GetRttiNameFromObject(void* ptr);
