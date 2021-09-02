/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Network/NetUtils.h"
#include "Core/Network/NetHttpRequest.h"

bool GetMachineIPv4(NetIPAddress& Output, bool GetPublicAddress)
{
    // For public ip address we query and external webapi.
    if (GetPublicAddress)
    {
        NetHttpRequest Request;
        Request.SetMethod(NetHttpMethod::GET);
        Request.SetUrl("http://api.ipify.org");
        if (Request.Send())
        {
            if (std::shared_ptr<NetHttpResponse> Response = Request.GetResponse(); Response && Response->GetWasSuccess())
            {
                std::string IPAddress;
                IPAddress.assign((char*)Response->GetBody().data(), Response->GetBody().size());

                if (!NetIPAddress::ParseString(IPAddress, Output))
                {
                    return false;
                }

                return true;
            }
        }
    }
    // Otherwise we can just grab one with some winsock shenanigans. This won't
    // work correctly for machines with multiple interfaces. But for our purposes it should
    // be ok. This path is mainly used for debugging.
    else
    {
        char Buffer[1024];
        if (gethostname(Buffer, sizeof(Buffer)) == SOCKET_ERROR)
        {
            return false;
        }

        struct hostent* HostEntry = gethostbyname(Buffer);
        if (!HostEntry)
        {
            return false;
        }

        struct in_addr* Addr = (struct in_addr* )HostEntry->h_addr;

        Output = NetIPAddress(
            Addr->S_un.S_un_b.s_b1,
            Addr->S_un.S_un_b.s_b2,
            Addr->S_un.S_un_b.s_b3,
            Addr->S_un.S_un_b.s_b4
        );

        return true;
    }

    return false;
}