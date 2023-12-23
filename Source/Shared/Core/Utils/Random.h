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

// Some general purpose random functionality.

void FillRandomBytes(std::vector<uint8_t>& Output);
void FillRandomBytes(uint8_t* Buffer, int Count);

double FRandRange(double min, double max);

std::string RandomName();
std::string RandomPassword();