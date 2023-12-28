/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/GameService/Utils/Data/PropertyList.h"
#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

bool PropertyListParser::Parse(const std::vector<uint8_t>& data, PropertyList& output)
{
    m_offset = 0;
    m_data = &data;
    m_output = &output;

    try
    {
        static size_t counter = 0;

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Z:\\ds3os\\Temp\\BloodMessageData\\%i.dat", counter++);

        WriteBytesToFile(buffer, data);

        while (m_offset < data.size())
        {
            uint32_t entry_size = Read<uint32_t>();
            uint32_t entry_id = Read<uint32_t>();
            
            WarningS("", "Property: %u", entry_size);

            m_offset += entry_size + 8;
        }
    }
    catch (std::out_of_range)
    {
        return false;
    }

    return true;
}
