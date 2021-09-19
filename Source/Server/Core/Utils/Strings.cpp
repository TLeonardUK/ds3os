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

bool IsCharRenderable(char c)
{
    return c >= 32 && c <= 126;
}

std::string BytesToString(const std::vector<uint8_t>& Bytes, const std::string& LinePrefix)
{
    static int column_width = 16;

    std::string result = "";

    for (size_t i = 0; i < Bytes.size(); i += column_width)
    {
        std::string hex = "";
        std::string chars = "";

        for (size_t r = i; r < i + column_width && r < Bytes.size(); r++)
        {
            uint8_t Byte = Bytes[r];

            hex += StringFormat("%02X ", Byte);
            chars += IsCharRenderable((char)Byte) ? (char)Byte : '.';
        }

        result += StringFormat("%s%-48s \xB3 %s\n", LinePrefix.c_str(), hex.c_str(), chars.c_str());
    }

    return result;
}
