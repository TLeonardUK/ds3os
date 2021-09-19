/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Network/NetIPAddress.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

bool NetIPAddress::FromHostname(const std::string& Input, NetIPAddress& Output)
{
    // TODO: Do a non-windows variant of this.

    struct addrinfo hints;
    struct addrinfo* result = nullptr;
    struct addrinfo* ptr = nullptr;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int Result = getaddrinfo(Input.c_str(), nullptr, &hints, &result);
    if (Result != 0)
    {
        return false;
    }

    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) 
    {
        if (ptr->ai_family != AF_INET)
        {
            continue;
        }

        struct sockaddr_in* addr_in = (struct sockaddr_in*)ptr->ai_addr;
        Output = NetIPAddress(
            addr_in->sin_addr.S_un.S_un_b.s_b1,
            addr_in->sin_addr.S_un.S_un_b.s_b2,
            addr_in->sin_addr.S_un.S_un_b.s_b3,
            addr_in->sin_addr.S_un.S_un_b.s_b4
        );
        return true;
    }

    return false;
}