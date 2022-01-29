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
void WriteLog(ConsoleColor Color, const char* Source, const char* Level, const char* Format, ...);

#define LOGGER_ARGS(...) , ##__VA_ARGS__
// Various macros for different log levels.
#if defined(_DEBUG)
#define Verbose(Format, ...)                    WriteLog(ConsoleColor::Grey,      "", "Verbose", Format, ## __VA_ARGS__);
#else
#define Verbose(Format, ...)           
#endif
#define Log(Format, ...)                        WriteLog(ConsoleColor::Grey,      "", "Log", Format, ## __VA_ARGS__);
#define Success(Format, ...)                    WriteLog(ConsoleColor::Green,     "", "Success", Format, ## __VA_ARGS__);
#define Warning(Format, ...)                    WriteLog(ConsoleColor::Yellow,    "", "Warning", Format, ## __VA_ARGS__);
#define LogError(Format, ...)                      WriteLog(ConsoleColor::Red,       "", "Error", Format, ## __VA_ARGS__);
#define Fatal(Format, ...)                      WriteLog(ConsoleColor::Red,       "", "Fatal", Format, ## __VA_ARGS__); Ensure(false);

// Same as the ones above but allows you to define the values of the "Source" column.
#if defined(_DEBUG)
#define VerboseS(Source, Format, ...)           WriteLog(ConsoleColor::Grey,      Source, "Verbose", Format, ## __VA_ARGS__);
#else
#define VerboseS(Source, Format, ...)           
#endif
#define LogS(Source, Format, ...)               WriteLog(ConsoleColor::Grey,      Source, "Log", Format, ## __VA_ARGS__);
#define SuccessS(Source, Format, ...)           WriteLog(ConsoleColor::Green,     Source, "Success", Format, ## __VA_ARGS__);
#define WarningS(Source, Format, ...)           WriteLog(ConsoleColor::Yellow,    Source, "Warning", Format, ## __VA_ARGS__);
#define ErrorS(Source, Format, ...)             WriteLog(ConsoleColor::Red,       Source, "Error", Format, ## __VA_ARGS__);
#define FatalS(Source, Format, ...)             WriteLog(ConsoleColor::Red,       Source, "Fatal", Format, ## __VA_ARGS__); Ensure(false);

#ifdef _WIN32
#define debug_break() __debugbreak()
#else
#include <csignal>
#define debug_break() (raise(SIGTRAP))
#endif

// Some general purpose debugging/assert macros.
#define Ensure(expr)                                \
    if (!(expr))                                    \
    {                                               \
        LogError("Check Failed: " #expr);              \
        debug_break();                              \
    }                                               
