// Dark Souls 3 - Open Server

#pragma once

#include <filesystem>
#include <string>

// Some general purpose IO functionality.

bool ReadTextFromFile(const std::filesystem::path& path, std::string& Output);
bool WriteTextToFile(const std::filesystem::path& path, const std::string& Input);

bool ReadBytesFromFile(const std::filesystem::path& path, std::vector<uint8_t>& Output);
bool WriteBytesToFile(const std::filesystem::path& path, const std::vector<uint8_t>& Input);
