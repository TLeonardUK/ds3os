/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>

struct Frpg2UdpPacket
{
public:

    // Length is equal to the rest of the payload minus the header.
    std::vector<uint8_t> Payload;

};