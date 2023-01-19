/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

// Some general purpose IO functionality.

bool ReadTextFromFile(const std::filesystem::path& path, std::string& Output);
bool WriteTextToFile(const std::filesystem::path& path, const std::string& Input);

bool ReadBytesFromFile(const std::filesystem::path& path, std::vector<uint8_t>& Output);
bool WriteBytesToFile(const std::filesystem::path& path, const std::vector<uint8_t>& Input);
