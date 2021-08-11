/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Utils/Compression.h"

#include "zlib.h"

bool Compress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    Output.resize(compressBound(Input.size()));

    uLongf DestinationLength = Output.size();

    if (compress(Output.data(), &DestinationLength, Input.data(), Input.size()) != Z_OK)
    {
        return false;
    }

    Output.resize(DestinationLength);

    return true;
}

bool Decompress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output, uint32_t DecompressedSize)
{
    Output.resize(DecompressedSize);

    uLongf DestinationLength = Output.size();

    if (uncompress(Output.data(), &DestinationLength, Input.data(), Input.size()) != Z_OK)
    {
        return false;
    }

    Output.resize(DestinationLength);

    return true;
}