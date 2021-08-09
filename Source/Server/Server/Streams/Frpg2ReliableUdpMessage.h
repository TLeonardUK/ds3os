// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

// See ds3server_packet.bt for commentry on what each of these
// fields appears to represent.
#pragma pack(push,1)
struct Frpg2ReliableUdpMessageHeader
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

struct Frpg2ReliableUdpPayloadHeader
{
public:
    uint32_t packet_id = 0; // TODO: Not sure about this at all, just testing. Adjust as we see more headers.
    uint32_t unknown_2 = 0;
    uint32_t unknown_3 = 0;
    uint8_t  unknown_4 = 0;

    void SwapEndian()
    {
    }
};
#pragma pack(pop)

static_assert(sizeof(Frpg2ReliableUdpPayloadHeader) == 13, "Message payload header is not expected size.");

struct Frpg2ReliableUdpMessage
{
public:
    Frpg2ReliableUdpMessageHeader Header;

    // Only used if fragment_index == 0
    uint32_t PayloadDecompressedLength = 0;
   
    // Header thats starts the compressed payload and describes
    // the protobuf that follows it in the true payload.
    Frpg2ReliableUdpPayloadHeader PayloadHeader;

    // Length of this payload should be the same
    // as the value stored in Header.payload_length
    std::vector<uint8_t> Payload;

};