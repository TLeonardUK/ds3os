/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Platform/Platform.h"

#include <ctime>
#include <cstdarg>
#include <list>
#include <cstdio>

namespace 
{
    std::mutex RecentMessageMutex;
    std::list<LogMessage> RecentMessages;
    bool QuietLoggingEnabled = false;
};

void SetQuietLogging(bool enabled)
{
    QuietLoggingEnabled = enabled;
}

std::list<LogMessage> GetRecentLogs()
{
    std::scoped_lock lock(RecentMessageMutex);
    return RecentMessages;
}

void StoreRecentMessage(const LogMessage& Message)
{
    std::scoped_lock lock(RecentMessageMutex);

    RecentMessages.push_back(Message);

    // Only keep a small buffer of messages, trim old ones.
    if (RecentMessages.size() > 16)
    {
        RecentMessages.erase(RecentMessages.begin());
    }
}

void WriteLogStatic(ConsoleColor Color, const char* Source, const char* Level, const char* Log)
{
    char buffer[256];
    char* buffer_to_use = buffer;

    time_t current_time = time(0);
    struct tm current_time_tstruct = *localtime(&current_time);

    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %X", &current_time_tstruct);

    int ret = snprintf(buffer_to_use, 256, "%s \xB3 %-7s \xB3 %-35s \xB3 %s\n", time_buffer, Level, Source, Log);
    if (ret >= 256)
    {
        buffer_to_use = new char[ret + 1];
        snprintf(buffer_to_use, ret + 1, "%s \xB3 %-7s \xB3 %-35s \xB3 %s\n", time_buffer, Level, Source, Log);
    }

    WriteToConsole(Color, buffer_to_use);

    LogMessage Message;
    Message.Level = Level;
    Message.Source = Source;
    Message.Message = Log;
    Message.Time = GetSeconds();
    StoreRecentMessage(Message);

    if (buffer_to_use != buffer)
    {
        delete[] buffer_to_use;
    }
}

void WriteLog(bool QuietLoggable, ConsoleColor Color, const char* Source, const char* Level, const char* Format, ...)
{
    if (QuietLoggingEnabled && !QuietLoggable)
    {
        return;
    }

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

    WriteLogStatic(Color, Source, Level, buffer_to_use);

    if (buffer_to_use != buffer)
    {
        delete[] buffer_to_use;
    }

    va_end(list);
}
