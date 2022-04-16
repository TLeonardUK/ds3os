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

#include <windows.h>
#include <WinSock2.h>
#include <chrono>

#if defined(_WIN32)
// Link to the windows socket library.
#pragma comment(lib, "Ws2_32.lib")
#endif

struct Win32CtrlSignalHandler
{
public:

    static BOOL WINAPI CtrlSignalCallback(DWORD dwCtrlType)
    {
        // We don't need to check dwCtrlType, we want to react to all of them really.
        PlatformEvents::OnCtrlSignal.Broadcast();
        return true;
    }

    Win32CtrlSignalHandler()
    {
        PlatformEvents::OnCtrlSignal.HookFirstRegistered([]() {
            SetConsoleCtrlHandler(CtrlSignalCallback, true);
        });
        PlatformEvents::OnCtrlSignal.HookLastUnregistered([]() {
            SetConsoleCtrlHandler(CtrlSignalCallback, false);
        });
    }

    ~Win32CtrlSignalHandler()
    {
        PlatformEvents::OnCtrlSignal.UnhookFirstRegistered();
        PlatformEvents::OnCtrlSignal.UnhookLastUnregistered();
    }

} gWin32CtrlSignalHandler;

bool PlatformInit()
{
    WSADATA wsaData;
    if (int Result = WSAStartup(MAKEWORD(2, 2), &wsaData); Result != 0) 
    {
        Error("WSAStartup failed with error 0x%08x.", Result);
        return false;
    }

    // Disable quick-edit on the output console. It results in the application
    // pausing if the user tries to select something in the window. Causes more
    // problems than it solves ...
    DWORD previousMode;
    HANDLE hStdout = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdout, &previousMode);
    SetConsoleMode(hStdout, ENABLE_EXTENDED_FLAGS | (previousMode & ~ENABLE_QUICK_EDIT_MODE));

    return true;
}

bool PlatformTerm()
{
    if (int Result = WSACleanup(); Result != 0)
    {
        Error("WSACleanup failed with error 0x%08x.", Result);
        return false;
    }

    return true;
}

void WriteToConsole(ConsoleColor Color, const char* Message)
{
    static HANDLE ConsoleHandle = INVALID_HANDLE_VALUE;
    if (ConsoleHandle == INVALID_HANDLE_VALUE)
    {
        ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    static WORD ColorTextAttributes[(int)ConsoleColor::Count] = {
        12, // Red
        14, // Yellow
        10, // Green
        15, // White
        07  // Grey
    };

    SetConsoleTextAttribute(ConsoleHandle, ColorTextAttributes[(int)Color]);

    OutputDebugStringA(Message);
    printf("%s", Message);
}

double GetSeconds()
{
    return (double)GetTickCount64() / 1000.0;
}

double GetHighResolutionSeconds()
{
    static bool RetrievedFrequency = false;
    static LARGE_INTEGER Frequency;
    static LARGE_INTEGER Epoch;
    if (!RetrievedFrequency)
    {
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&Epoch);
        RetrievedFrequency = true;
    }

    LARGE_INTEGER Current;
    QueryPerformanceCounter(&Current);

    LARGE_INTEGER ElapsedMicroseconds;
    ElapsedMicroseconds.QuadPart = Current.QuadPart - Epoch.QuadPart;
    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    return (double)ElapsedMicroseconds.QuadPart / 1000000.0;
}
