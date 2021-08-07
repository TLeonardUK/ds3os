// Dark Souls 3 - Open Server

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