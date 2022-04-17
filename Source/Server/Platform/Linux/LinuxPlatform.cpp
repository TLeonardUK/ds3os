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

#include <csignal>
#include <cstdio>

struct LinuxCtrlSignalHandler 
{
public:
    static void CtrlSignalCallback(int _signo) 
    {
        PlatformEvents::OnCtrlSignal.Broadcast();
    }

    LinuxCtrlSignalHandler() 
    {
        PlatformEvents::OnCtrlSignal.HookFirstRegistered([]() { 
            signal(SIGINT, CtrlSignalCallback); 
        });
        PlatformEvents::OnCtrlSignal.HookLastUnregistered([]() { 
            signal(SIGINT, SIG_DFL); 
        });
    }

    ~LinuxCtrlSignalHandler() 
    {
        PlatformEvents::OnCtrlSignal.UnhookFirstRegistered();
        PlatformEvents::OnCtrlSignal.UnhookLastUnregistered();
    }

} gLinuxCtrlSignalHandler;

bool PlatformInit() 
{ 
    return true; 
}

bool PlatformTerm() 
{ 
    return true; 
}

void WriteToConsole(ConsoleColor Color, const char *Message) 
{
    printf("%s", Message);
}

double GetSeconds() 
{
    struct timespec ts {};
#if !defined _POSIX_MONOTONIC_CLOCK || _POSIX_MONOTONIC_CLOCK < 0
    clock_gettime(CLOCK_REALTIME, &ts);
#elif _POSIX_MONOTONIC_CLOCK > 0
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) 
    {
        clock_gettime(CLOCK_REALTIME, &ts);
    }
#endif

    return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}


double GetHighResolutionSeconds()
{
    return GetSeconds();
}
