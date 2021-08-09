// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

struct Frpg2UdpPacket
{
public:

    // Length is equal to the rest of the payload minus the header.
    std::vector<uint8_t> Payload;

};