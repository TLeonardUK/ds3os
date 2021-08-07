// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

// See ds3server_packet.bt for commentry on what each of these
// fields appears to represent.
#pragma pack(push,1)
struct Frpg2MessageHeader
{
public:

    uint32_t header_size            = 12; // As far as I can tell this is always 12.
    uint32_t unknown_1              = 0x00;               
    uint32_t request_index          = 0x00; // Request/response messages have the same value for this.

    void SwapEndian()
    {
        header_size = HostOrderToBigEndian(header_size);
        unknown_1 = HostOrderToBigEndian(unknown_1);
        request_index = HostOrderToLittleEndian(request_index);
    }
};

struct Frpg2MessageResponseHeader
{
public:

    uint32_t unknown_1 = 0x0;
    uint32_t unknown_2 = 0x1;
    uint32_t unknown_3 = 0x0;
    uint32_t unknown_4 = 0x0;

    void SwapEndian()
    {
        unknown_1 = HostOrderToBigEndian(unknown_1);
        unknown_2 = HostOrderToBigEndian(unknown_2);
        unknown_3 = HostOrderToBigEndian(unknown_3);
        unknown_4 = HostOrderToBigEndian(unknown_4);
    }
};
#pragma pack(pop)

static_assert(sizeof(Frpg2MessageHeader) == 12, "Message header is not expected size.");

struct Frpg2Message
{
public:

    Frpg2MessageHeader Header;
    Frpg2MessageResponseHeader ResponseHeader; // Only exists if its a response :thinking:

    // Length of this payload should be the same
    // as the value stored in Header.payload_length
    std::vector<uint8_t> Payload;

};