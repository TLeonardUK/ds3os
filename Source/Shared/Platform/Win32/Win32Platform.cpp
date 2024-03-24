/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include <DbgEng.h>
#include <DbgHelp.h>

#include "Platform/Platform.h"
#include "Core/Utils/Logging.h"

#include <windows.h>
#include <WinSock2.h>
#include <chrono>

#include <array>

#include <Rpc.h>

#if defined(_WIN32)
// Link to the windows socket library.
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "rpcrt4.lib")
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

LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    Log("================== Crash Log ==================");

    std::unique_ptr<Callstack> Stack = CaptureCallstack(1);
    for (auto& Frame : Stack->Frames)
    {
        Log("0x%016zx %-50s %s@%zi", 
            Frame.Address, 
            Frame.Function.empty() ? "<unknown>" : Frame.Function.c_str(), 
            Frame.Filename.empty() ? "<unknown>" : Frame.Filename.c_str(),
            Frame.Line
        );
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

bool PlatformInit()
{
    LoadSymbols();

    SetUnhandledExceptionFilter(ExceptionHandler);

    WSADATA wsaData;
    if (int Result = WSAStartup(MAKEWORD(2, 2), &wsaData); Result != 0) 
    {
        if (Result == 0x0000276b)
        {
            MessageBoxA(nullptr, "Failed to initialize networking layer.\n\nIf you are running a mod that uses modengine, make sure you set blocknetworkaccess to 0 in modengine.ini to enable network access.\n\nGame will continue but connecting to a server may not be possible until you resolve this issue.", "Network Access Blocked", 0);
        }
        Warning("WSAStartup failed with error 0x%08x. It may have already been initialized. Ignoring ...", Result);
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

    UnloadSymbols();

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

bool LoadSymbols()
{
    bool result = SymInitialize(GetCurrentProcess(), nullptr, true);
    if (!result)
    {
        Warning("Failed to load symbols for current process.");
    }
    
    return true;
}

bool UnloadSymbols()
{
    bool result = SymCleanup(GetCurrentProcess());
    if (!result)
    {
        Warning("Failed to unload symbols for current process.");
    }

    return result;
}

std::unique_ptr<Callstack> CaptureCallstack(size_t FrameOffset, size_t FrameCount)
{
    constexpr size_t k_max_frames = 256;
    std::array<void*, k_max_frames> frames;

    USHORT captured_frames = RtlCaptureStackBackTrace(
        static_cast<ULONG>(FrameOffset + 1), 
        static_cast<ULONG>(std::min(FrameCount, k_max_frames)),
        frames.data(),
        nullptr);

    std::unique_ptr<Callstack> result = std::make_unique<Callstack>();
    result->Frames.resize(captured_frames);

    HANDLE process = GetCurrentProcess();
    DWORD displacement = 0;
    DWORD64 displacement64 = 0;

    for (size_t i = 0; i < captured_frames; i++)
    {
        Callstack::Frame& frame = result->Frames[i];
        frame.Address = reinterpret_cast<size_t>(frames[i]);

        char buffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME * sizeof(TCHAR)];

        IMAGEHLP_SYMBOL64* symbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buffer);
        memset(buffer, 0, sizeof(buffer));
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
        symbol->MaxNameLength = MAX_SYM_NAME;
        if (SymGetSymFromAddr64(process, frame.Address, &displacement64, symbol))
        {
            frame.Function = symbol->Name;
        }

        IMAGEHLP_MODULE64* module = reinterpret_cast<IMAGEHLP_MODULE64*>(buffer);
        memset(buffer, 0, sizeof(buffer));
        module->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
        if (SymGetModuleInfo64(process, frame.Address, module))
        {
            frame.Module = module->ModuleName;
        }

        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(line);
        if (SymGetLineFromAddr64(process, frame.Address, &displacement, &line))
        {
            frame.Filename = line.FileName;
            frame.Line = line.LineNumber;
        }
    }

    return std::move(result);
}

std::string MakeGUID()
{
    UUID uuid;
    UuidCreate(&uuid);

    unsigned char* str;
    UuidToStringA(&uuid, &str);

    std::string result((char*)str);

    RpcStringFreeA(&str);

    return result;
}