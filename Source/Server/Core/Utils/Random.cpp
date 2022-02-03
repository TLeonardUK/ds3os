/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Utils/Random.h"

#include <random>

void FillRandomBytes(std::vector<uint8_t>& Output)
{
	static std::random_device device;
	static std::mt19937 generator(device());

	std::uniform_int_distribution<> distribution(0, 255);

	for (size_t i = 0; i < Output.size(); i++)
	{
		Output[i] = distribution(generator);
	}
}

void FillRandomBytes(uint8_t* Buffer, int Count)
{
	static std::random_device device;
	static std::mt19937 generator(device());

	std::uniform_int_distribution<> distribution(0, 255);

	for (size_t i = 0; i < Count; i++)
	{
		Buffer[i] = distribution(generator);
	}
}

double FRandRange(double min, double max)
{
	double range = max - min;
	double weight = (static_cast<double>(rand()) / RAND_MAX);
	return min + (weight * range);
}