/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <vector>
#include <stdexcept>

class PropertyList
{
public:
    

};

class PropertyListParser
{
public:
    bool Parse(const std::vector<uint8_t>& data, PropertyList& output);

protected:
    template <typename T>
    T Read()
    {
        size_t bytes_remaining = m_data->size() - m_offset;
        if (bytes_remaining < sizeof(T))
        {
            throw new std::out_of_range("Tried to read out of bounds of property list data.");
        }
        m_offset += sizeof(T);
        return *reinterpret_cast<const T*>(m_data->data() + m_offset);
    }

private:
    size_t m_offset = 0;
    const std::vector<uint8_t>* m_data = nullptr;
    PropertyList* m_output;

};