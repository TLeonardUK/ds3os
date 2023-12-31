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

// Generates a hex editor style layout.
std::string BytesToString(const std::vector<uint8_t>& Bytes, const std::string& LinePrefix);

/** @todo */
template<typename... Args>
std::string StringFormat(const char* format, Args... args)
{
    size_t size = static_cast<size_t>(snprintf(nullptr, 0, format, args ...)) + 1; // Extra space for '\0'

    std::unique_ptr<char[]> buffer(new char[size]);
    snprintf(buffer.get(), size, format, args ...);

    return std::string(buffer.get(), buffer.get() + size - 1); // We don't want the '\0' inside
}

std::string TrimString(const std::string& input);

#ifdef _WIN32

//  Converts a wide utf-16 string to utf-8.
std::string NarrowString(const std::wstring& input);

//  Converts a utf-8 string to a utf-16 string.
std::wstring WidenString(const std::string& input);

#endif

//  Determines if a given string ends with another string.
bool StringEndsWith(const std::string& subject, const std::string& needle);

//  Determines if a given string starts with another string.
bool StringStartsWith(const std::string& subject, const std::string& needle);

// Returns true if all the characters in a string are human readable.
bool StringIsHumanReadable(const std::string& subject);