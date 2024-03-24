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
#include <uuid/uuid.h>
#include <execinfo.h>
#include <signal.h>
#include <cstring>
#include <cxxabi.h>
#include <link.h>
#include <dlfcn.h>
#include <sstream>

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

void CrashSignalHandler(int signal)
{
    Log("================== Crash Log ==================");
    Log("Signal: %i", signal);
    Log("");

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

    exit(1);
}

bool PlatformInit() 
{ 
    signal(SIGSEGV, CrashSignalHandler);
    signal(SIGBUS,  CrashSignalHandler);
    signal(SIGFPE,  CrashSignalHandler);
    signal(SIGILL,  CrashSignalHandler);
    signal(SIGABRT, CrashSignalHandler);
    signal(SIGSYS,  CrashSignalHandler);
    signal(SIGXCPU, CrashSignalHandler);
    signal(SIGXFSZ, CrashSignalHandler);

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

bool LoadSymbols()
{
    return true;
}

bool UnloadSymbols()
{
    return true;
}

bool exec(const char* cmd, std::string& result) 
{
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) 
    {
        return false;
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) 
    {
        result += buffer.data();
    }
    return true;
}

size_t GetVmaAddress(size_t Addr)
{
    Dl_info info;
    struct link_map* link_map;
    dladdr1((void*)Addr, &info, (void**)&link_map, RTLD_DL_LINKMAP);
    return Addr - link_map->l_addr;
}

std::unique_ptr<Callstack> CaptureCallstack(size_t FrameOffset, size_t FrameCount)
{
    void* FramePointers[128];
    size_t MaxFrames = std::min((size_t)128, FrameCount);
    size_t FramesCaptured = backtrace(FramePointers, MaxFrames);

    std::unique_ptr<Callstack> result = std::make_unique<Callstack>();
    if (FramesCaptured <= FrameOffset)
    {
        return result;
    }

    char** FrameSymbols = backtrace_symbols(FramePointers, FramesCaptured);    

    result->Frames.resize(FramesCaptured - FrameOffset);
    for (size_t i = FrameOffset; i < FramesCaptured; i++)
    {        
        Callstack::Frame& frame = result->Frames[i - FrameOffset];

        frame.Address = GetVmaAddress(reinterpret_cast<size_t>(FramePointers[i]));
        frame.Function = "";
        frame.Module = "";
        frame.Filename = "";
        frame.Line = 0;

        // Get module for resolving filename/line 
        Dl_info info;
        if (dladdr(FramePointers[i], &info))
        {            
            // Try and resolve filename/line.
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "addr2line -e %s -Cis %zx", info.dli_fname, frame.Address);

            std::string cmdOutput;
            if (exec(cmd, cmdOutput))
            {
                if (const char* Ptr = strchr(cmdOutput.c_str(), ':'); Ptr != nullptr)
                {
                    frame.Filename.assign(cmdOutput.c_str(), std::distance(cmdOutput.c_str(), Ptr));
                    if (frame.Filename == "??")
                    {
                        frame.Filename = "";
                    }

                    frame.Line = atoi(Ptr + 1);
                }
            }
        }
        
        // Extract module
        const char* SymbolPointer = FrameSymbols[i];
        if (const char* Ptr = strchr(SymbolPointer, '('); Ptr != nullptr)
        {
            frame.Module.assign(SymbolPointer, std::distance(SymbolPointer, Ptr));
            SymbolPointer = Ptr + 1;
        }
        // Extract and demangle function.
        if (const char* Ptr = strchr(SymbolPointer, ')'); Ptr != nullptr)
        {
            if (const char* PlusPtr = strchr(SymbolPointer, '+'); PlusPtr != nullptr && PlusPtr < Ptr)
            {
                Ptr = PlusPtr;
            }

            size_t NameLength = std::distance(SymbolPointer, Ptr);
            if (NameLength >= 1)
            {
                frame.Function.assign(SymbolPointer, NameLength);

                // Demangle the symbol.
                int Status = 0;
                const char* DemangledName = abi::__cxa_demangle(frame.Function.c_str(), nullptr, 0, &Status);
                if (DemangledName && Status == 0)
                {
                    frame.Function = DemangledName;
                }                
            }
            SymbolPointer = Ptr + 1;
        }
    }

    return result;
}

std::string MakeGUID()
{
    uuid_t uuid;
    uuid_generate_random(uuid);
    char result[37];
    uuid_unparse(uuid, result);
    return result;
}