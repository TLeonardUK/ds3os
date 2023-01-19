/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/Strings.h"

#include <sstream>
#include <iomanip>
#include <string>

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
    static int column_width = 32;

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

        result += StringFormat("%s%-97s \xB3 %s\n", LinePrefix.c_str(), hex.c_str(), chars.c_str());
    }

    return result;
}


std::string TrimString(const std::string& input)
{
    size_t startWhiteCount = 0;
    size_t endWhiteCount = 0;

    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] < 32)
        {
            startWhiteCount++;
        }
        else
        {
            break;
        }
    }

    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[input.size() - (i + 1)] < 32)
        {
            endWhiteCount++;
        }
        else
        {
            break;
        }
    }

    if (startWhiteCount + endWhiteCount >= input.size())
    {
        return "";
    }

    return input.substr(startWhiteCount, input.size() - startWhiteCount - endWhiteCount);
}