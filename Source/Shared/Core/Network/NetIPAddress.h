/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <cstdio>
#include <cstdint>

// Simple wrapper for an IPv4 address.

class NetIPAddress
{
public:
    NetIPAddress()
    {
        Bytes[0] = 127;
        Bytes[1] = 0;
        Bytes[2] = 0;
        Bytes[3] = 1;
    }

    NetIPAddress(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
    {
        Bytes[0] = b1;
        Bytes[1] = b2;
        Bytes[2] = b3;
        Bytes[3] = b4;
    }

    bool IsPrivateNetwork()
    {
        return 
            Bytes[0] == 127 ||                                                      // 127.*.*.*
            Bytes[0] == 10 ||                                                       // 10.*.*.*
            (Bytes[0] == 172 && Bytes[1] >= 16 && Bytes[1] <= 31) ||                // 172.16.*.* -> 172.31.*.*
            (Bytes[0] == 192 && Bytes[1] == 168);                                   // 192.168.*.*
    }

    std::string ToString()
    {
        char Buffer[64];
        snprintf(Buffer, 64, "%u.%u.%u.%u", Bytes[0], Bytes[1], Bytes[2], Bytes[3]);
        return Buffer;
    }

    static bool ParseString(const std::string& Input, NetIPAddress& Output)
    {
        uint8_t a, b, c, d;
        int Ret = sscanf(Input.data(), "%hhd.%hhd.%hhd.%hhd", &a, &b, &c, &d);
        if (Ret == 4)
        {
            Output = NetIPAddress(a, b, c, d);
            return true;
        }
        return false;
    }

    static bool FromHostname(const std::string& Input, NetIPAddress& Output);

private:
    uint8_t Bytes[4];

};
