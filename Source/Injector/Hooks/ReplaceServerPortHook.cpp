/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Hooks/ReplaceServerPortHook.h"
#include "Injector/Injector/Injector.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "ThirdParty/detours/src/detours.h"

#include <vector>
#include <iterator>
#include <WinSock2.h>

namespace 
{
    using connect_p = int(WSAAPI*)(SOCKET s, const sockaddr* name, int namelen);
    connect_p s_original_connect;

    int WSAAPI ConnectHook(SOCKET s, const sockaddr* name, int namelen)
    {
        sockaddr_in* addr = (sockaddr_in*)name;

        if (addr->sin_port == htons(50050))
        {
            const RuntimeConfig& Config = Injector::Instance().GetConfig();

            Log("Attempt to connect to login server, patching port to '%i'.", Config.ServerPort);
            addr->sin_port = ntohs(Config.ServerPort);
        }

        return s_original_connect(s, name, namelen);
    }
};

bool ReplaceServerPortHook::Install(Injector& injector)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    s_original_connect = reinterpret_cast<connect_p>(connect);
    DetourAttach(&(PVOID&)s_original_connect, ConnectHook);

    DetourTransactionCommit();

    return true;
}

void ReplaceServerPortHook::Uninstall()
{

}

const char* ReplaceServerPortHook::GetName()
{
    return "Replace Server Port";
}

