// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

// See ds3server_packet.bt for commentry on what each of these
// fields appears to represent.
#pragma pack(push,1)
struct Frpg2UdpPacketHeader
{
public:

    uint64_t auth_token;
};
#pragma pack(pop)

static_assert(sizeof(Frpg2UdpPacketHeader) == 8, "Udp packet header is not expected size.");

struct Frpg2UdpPacket
{
public:

    Frpg2UdpPacketHeader Header;

    // Length is equal to the rest of the payload minus the header.
    std::vector<uint8_t> Payload;

};