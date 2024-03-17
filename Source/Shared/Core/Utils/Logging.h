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

#include "Shared/Platform/Platform.h"

#include <list>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

struct LogMessage
{
    double Time;
    std::string Source;
    std::string Level;
    std::string Message;
};

// Disbale all but error messages.
void SetQuietLogging(bool enabled);

// Gets the most recent log messages, so they can be displayed in webui or other places.
std::list<LogMessage> GetRecentLogs();

// Writes a given entry into the output log.
void WriteLog(bool QuietLoggable, ConsoleColor Color, const char* Source, const char* Level, const char* Format, ...);

// Various macros for different log levels.
#if defined(_DEBUG)
#define Verbose(Format, ...)                    WriteLog(false, ConsoleColor::Grey,      "", "Verbose", Format, ##__VA_ARGS__);
#else
#define Verbose(Format, ...)           
#endif
#define Log(Format, ...)                        WriteLog(false, ConsoleColor::Grey,      "", "Log", Format, ##__VA_ARGS__);
#define Success(Format, ...)                    WriteLog(true, ConsoleColor::Green,     "", "Success", Format, ##__VA_ARGS__);
#define Warning(Format, ...)                    WriteLog(true, ConsoleColor::Yellow,    "", "Warning", Format, ##__VA_ARGS__);
#define Error(Format, ...)                      WriteLog(true, ConsoleColor::Red,       "", "Error", Format, ##__VA_ARGS__);
#define Fatal(Format, ...)                      WriteLog(true, ConsoleColor::Red,       "", "Fatal", Format, ##__VA_ARGS__); Ensure(false);

// Same as the ones above but allows you to define the values of the "Source" column.
#if defined(_DEBUG)
#define VerboseS(Source, Format, ...)           WriteLog(false, ConsoleColor::Grey,      Source, "Verbose", Format, ##__VA_ARGS__);
#else
#define VerboseS(Source, Format, ...)           
#endif
#define LogS(Source, Format, ...)               WriteLog(false, ConsoleColor::Grey,      Source, "Log", Format, ##__VA_ARGS__);
#define SuccessS(Source, Format, ...)           WriteLog(true,  ConsoleColor::Green,     Source, "Success", Format, ##__VA_ARGS__);
#define WarningS(Source, Format, ...)           WriteLog(true, ConsoleColor::Yellow,    Source, "Warning", Format, ##__VA_ARGS__);
#define ErrorS(Source, Format, ...)             WriteLog(true, ConsoleColor::Red,       Source, "Error", Format, ##__VA_ARGS__);
#define FatalS(Source, Format, ...)             WriteLog(true, ConsoleColor::Red,       Source, "Fatal", Format, ##__VA_ARGS__); Ensure(false);

#ifdef _WIN32
#define Breakpoint() __debugbreak()
#else
#include <csignal>
#define Breakpoint() raise(SIGTRAP)
#endif

// Some general purpose debugging/assert macros.
#define Ensure(expr)                                \
    if (!(expr))                                    \
    {                                               \
        Error("Check Failed: " #expr);              \
        Breakpoint();                               \
    }                                               
