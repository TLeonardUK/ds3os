/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

std::string BytesToHex(const std::vector<uint8_t>& Bytes);

/** @todo */
template<typename... Args>
std::string StringFormat(const char* format, Args... args)
{
    size_t size = static_cast<size_t>(snprintf(nullptr, 0, format, args ...)) + 1; // Extra space for '\0'

    std::unique_ptr<char[]> buffer(new char[size]);
    snprintf(buffer.get(), size, format, args ...);

    return std::string(buffer.get(), buffer.get() + size - 1); // We don't want the '\0' inside
}
