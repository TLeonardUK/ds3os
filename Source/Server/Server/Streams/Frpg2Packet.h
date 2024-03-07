/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Shared/Core/Utils/Endian.h"

#include <vector>
#include <string>
#include <cstdint>

// See ds3server_packet.bt for commentry on what each of these
// fields appears to represent.
#pragma pack(push,1)
struct Frpg2PacketHeader
{
public:

    uint16_t send_counter;
    uint8_t  unknown_1              = 0x00;
    uint8_t  unknown_2              = 0x00;
    uint32_t payload_length;
    uint16_t unknown_3              = 0x00;
    uint16_t payload_length_short;

    void SwapEndian()
    {
        send_counter = HostOrderToBigEndian(send_counter);
        payload_length = HostOrderToBigEndian(payload_length);
        payload_length_short = HostOrderToBigEndian(payload_length_short);
    }
};
#pragma pack(pop)

static_assert(sizeof(Frpg2PacketHeader) == 12, "Packet header is not expected size.");

struct Frpg2Packet
{
public:

    Frpg2PacketHeader Header;

    // Length of this payload should be the same
    // as the value stored in Header.payload_length
    std::vector<uint8_t> Payload;

    std::string Disassembly;

};