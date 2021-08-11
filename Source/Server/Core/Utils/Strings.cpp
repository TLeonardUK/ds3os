/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Utils/Strings.h"

#include <sstream>
#include <iomanip>

std::string BytesToHex(const std::vector<uint8_t>& Bytes)
{
    std::stringstream ss;
    for (uint8_t Value : Bytes)
    {
        ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)Value;
        //ss << " ";
    }

    return ss.str();
}