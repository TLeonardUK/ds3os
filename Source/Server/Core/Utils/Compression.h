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

bool Compress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output);
bool Decompress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output, uint32_t DecompressedSize);