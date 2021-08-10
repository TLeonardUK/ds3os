// Dark Souls 3 - Open Server

#pragma once

#include <filesystem>
#include <string>

bool Compress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output);
bool Decompress(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output, uint32_t DecompressedSize);