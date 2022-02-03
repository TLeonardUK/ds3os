/*
* Dark Souls 3 - Open Server
* Copyright (C) 2021 Tim Leonard
*
* This program is free software; licensed under the MIT license.
* You should have received a copy of the license along with this program.
* If not, see <https://opensource.org/licenses/MIT>.
*/

#include "Platform/Platform.h"
#include "Core/Utils/Logging.h"

#include <cstdio>
#include <csignal>

struct LinuxCtrlSignalHandler
{
public:

  static void CtrlSignalCallback(int _signo)
  {
    // We don't need to check dwCtrlType, we want to react to all of them really.
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

void WriteToConsole(ConsoleColor Color, const char* Message)
{
  static int ColorTextAttributes[(int)ConsoleColor::Count] = {
      12, // Red
      14, // Yellow
      10, // Green
      15, // White
      07  // Grey
  };

  printf("%s", Message);
}

double GetSeconds()
{
  struct timespec ts{};
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    return 1.0;
  }

  return (double)ts.tv_sec / 1000.0;
}
