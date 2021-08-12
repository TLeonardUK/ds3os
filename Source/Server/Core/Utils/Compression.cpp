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
    uLongf DestinationLength = Output.size();

    Output.resize(compressBound(Input.size()));

    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = Input.size();
    defstream.next_in = (Bytef*)Input.data();
    defstream.avail_out = Output.size();
    defstream.next_out = (Bytef*)Output.data();

    // Match these settings EXACTLY or ds3 has a fit. I think its doing a hard-check on the header
    // generated which changes based on the settings.
    if (deflateInit2(&defstream, 7, Z_DEFLATED, 13, 9, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        return false;
    }
    if (deflate(&defstream, Z_FINISH) != Z_STREAM_END)
    {
        return false;
    }
    if (deflateEnd(&defstream) != Z_OK)
    {
        return false;
    }

    Output.resize(defstream.total_out);

    return true;
}

bool Decompress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output, uint32_t DecompressedSize)
{
    Output.resize(DecompressedSize);

    uLongf DestinationLength = Output.size();
    uLongf SourceLength = Input.size();

    if (uncompress2(Output.data(), &DestinationLength, Input.data(), &SourceLength) != Z_OK)
    {
        return false;
    }

    Output.resize(DestinationLength);

    return true;
}