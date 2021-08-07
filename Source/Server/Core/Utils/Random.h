// Dark Souls 3 - Open Server

#pragma once

#include <filesystem>
#include <string>

// Some general purpose random functionality.

void FillRandomBytes(std::vector<uint8_t>& Output);
void FillRandomBytes(uint8_t* Buffer, int Count);