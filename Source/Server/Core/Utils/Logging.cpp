/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Utils/Logging.h"
#include "Platform/Platform.h"

#include <ctime>
#include <cstdarg>

void WriteLogStatic(ConsoleColor Color, const char* Level, const char* Log)
{
    char buffer[256];
    char* buffer_to_use = buffer;

    time_t current_time = time(0);
    struct tm current_time_tstruct = *localtime(&current_time);

    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %X", &current_time_tstruct);

    int ret = snprintf(buffer_to_use, 256, "%s \xB3 %-7s \xB3 %s\n", time_buffer, Level, Log);
    if (ret >= 256)
    {
        buffer_to_use = new char[ret + 1];
        snprintf(buffer_to_use, ret + 1, "%s \xB3 %-7s \xB3 %s\n", time_buffer, Level, Log);
    }

    WriteToConsole(Color, buffer_to_use);

    if (buffer_to_use != buffer)
    {
        delete[] buffer_to_use;
    }
}

void WriteLog(ConsoleColor Color, const char* Level, const char* Format, ...)
{
    char buffer[256];
    char* buffer_to_use = buffer;

    va_list list;
    va_start(list, Format);

    int ret = vsnprintf(buffer_to_use, 256, Format, list);
    if (ret >= 256)
    {
        buffer_to_use = new char[ret + 1];
        vsnprintf(buffer_to_use, ret + 1, Format, list);
    }

    WriteLogStatic(Color, Level, buffer_to_use);

    if (buffer_to_use != buffer)
    {
        delete[] buffer_to_use;
    }

    va_end(list);
}
