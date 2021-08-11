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

#include <cstdarg>

void WriteLog(ConsoleColor Color, const char* Format, ...)
{
    char buffer[256];
    char* buffer_to_use = buffer;

    va_list list;
    va_start(list, Format);

    int ret = vsnprintf(buffer_to_use, 1024, Format, list);
    if (ret >= 256)
    {
        buffer_to_use = new char[ret + 1];
        vsnprintf(buffer_to_use, ret, Format, list);
        buffer_to_use[ret] = '\0';
    }

    WriteToConsole(Color, buffer_to_use);

    if (buffer_to_use != buffer)
    {
        delete[] buffer_to_use;
    }

    va_end(list);
}
