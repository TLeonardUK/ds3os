// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

// See ds3server_packet.bt for commentry on what each of these
// fields appears to represent.
#pragma pack(push,1)
struct Frpg2ReliableUdpFragmentHeader
{
public:
    uint16_t packet_counter       = 0;
    uint8_t  compress_flag        = 0;               
    uint8_t  unknown_1            = 0;
    uint8_t  unknown_2            = 0;               
    uint8_t  unknown_3            = 0;
    uint16_t total_payload_length = 0;
    uint8_t  unknown_4            = 0;
    uint8_t  fragment_index       = 0;
    uint16_t fragment_length      = 0;

    void SwapEndian()
    {
        total_payload_length = BigEndianToHostOrder(total_payload_length);
        fragment_length = BigEndianToHostOrder(fragment_length);
    }
};
#pragma pack(pop)

struct Frpg2ReliableUdpFragment
{
public:
    Frpg2ReliableUdpFragmentHeader Header;

    // TODO: Fix this, it breaks our abstraction, but I can't see any particularly nice
    // way of passing this around so we can send the appropriate DAT/DAT_ACK codes.
    uint32_t AckSequenceIndex = 0;

    // Only used if fragment_index == 0
    uint32_t PayloadDecompressedLength = 0;
   
    // Length of this payload should be the same
    // as the value stored in Header.payload_length
    std::vector<uint8_t> Payload;

};