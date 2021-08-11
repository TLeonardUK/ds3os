/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

// File contains some super simplistic logging support.
// Might be worth making something a bit more robust than this when time allows.

#pragma once

#include "Platform/Platform.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

// Writes a given entry into the output log.
void WriteLog(ConsoleColor Color, const char* Format, ...);

// Various macros for different log levels.
#define Log(Format, ...)        WriteLog(ConsoleColor::Grey,      "%-80s " Format "\n", "[" __FILE__ ":" STRINGIFY(__LINE__) "] ", __VA_ARGS__);
#define Success(Format, ...)    WriteLog(ConsoleColor::Green,     "%-80s " Format "\n", "[" __FILE__ ":" STRINGIFY(__LINE__) "] ", __VA_ARGS__);
#define Warning(Format, ...)    WriteLog(ConsoleColor::Yellow,    "%-80s Warning: " Format "\n", "[" __FILE__ ":" STRINGIFY(__LINE__) "] ", __VA_ARGS__);
#define Error(Format, ...)      WriteLog(ConsoleColor::Red,       "%-80s Error: " Format "\n", "[" __FILE__ ":" STRINGIFY(__LINE__) "] ", __VA_ARGS__);

// Some general purpose debugging/assert macros.
#define Ensure(expr)                                \
    if (!expr)                                      \
    {                                               \
        Error("Check Failed: " #expr);              \
        __debugbreak();                             \
    }                                               
