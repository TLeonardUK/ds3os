/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/Random.h"

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

std::string RandomName()
{
    const char* Name[] = {
        "Jay",
        "Jellyfish",
        "Junglefowl",
        "Kangaroo",
        "Kingfisher",
        "Kite",
        "Kiwi",
        "Koala",
        "Koi",
        "Krill",
        "Ladybug",
        "Lamprey",
        "Landfowl",
        "Lark",
        "Leech",
        "Lemming",
        "Lemur",
        "Leopard",
        "Leopon",
        "Limpet",
        "Lion",
        "Lizard",
        "Llama",
        "Lobster",
        "Locust",
        "Loon",
        "Louse",
        "Lungfish",
        "Lynx",
        "Macaw",
        "Mackerel",
        "Magpie",
        "Mammal",
        "Manatee",
        "Mandrill",
        "Marlin",
        "Marmoset",
        "Marmot",
        "Marsupial",
        "Marten",
        "Mastodon",
        "Meadowlark",
        "Meerkat",
        "Mink",
        "Minnow",
        "Mite",
        "Mockingbird",
        "Mole",
        "Mollusk",
        "Mongoose",
        "Monkey",
        "Moose",
        "Mosquito",
        "Moth",
        "Mouse",
        "Mule",
        "Muskox",
        "Narwhal",
        "Newt",
        "Nightingale",
    };

    const char* Adjective[] = {
        "Alert",
        "Alienated",
        "Alive",
        "All",
        "Altruistic",
        "Amazing",
        "Ambitious",
        "Ample",
        "Amused",
        "Amusing",
        "Anchored",
        "Ancient",
        "Angelic",
        "Angry",
        "Anguished",
        "Animated",
        "Annual",
        "Another",
        "Antique",
        "Anxious",
        "Any",
        "Apprehensive",
        "Appropriate",
        "Apt",
        "Arctic",
        "Arid",
        "Aromatic",
        "Artistic",
        "Ashamed",
        "Assured",
        "Astonishing",
        "Athletic",
        "Attached",
        "Attentive",
        "Attractive",
        "Austere",
        "Authentic",
        "Authorized",
        "Automatic",
        "Avaricious",
        "Average",
        "Aware",
        "Awesome",
        "Awful",
        "Awkward",
        "Babyish",
        "Bad",
        "Back",
        "Baggy",
        "Bare",
        "Barren",
        "Basic",
        "Beautiful",
        "Belated",
        "Beloved",
        "Beneficial",
        "Better",
        "Best",
        "Bewitched",
        "Big",
        "Biodegradable",
        "Bitter",
        "Black",
    };

    static std::random_device device;
    static std::mt19937 generator(device());

    size_t NameCount = sizeof(Name) / sizeof(*Name);
    size_t AdjectiveCount = sizeof(Adjective) / sizeof(*Adjective);

    std::uniform_int_distribution<> NameDistribution(0, (sizeof(Name) / sizeof(*Name)) - 1);
    std::uniform_int_distribution<> AdjectiveDistribution(0, (sizeof(Adjective) / sizeof(*Adjective)) - 1);

    char Buffer[256];
    snprintf(Buffer, sizeof(Buffer), "%s%s", Adjective[AdjectiveDistribution(generator)], Name[NameDistribution(generator)]);

    return Buffer;
}

std::string RandomPassword()
{
    static std::random_device device;
    static std::mt19937 generator(device());

    const char AllowChars[] = {
        'a', 'A',
        'b', 'B',
        'c', 'C',
        'd', 'D',
        'e', 'E',
        'f', 'F',
        'g', 'G',
        'h', 'H',
        'i', 'I',
        'j', 'J',
        'k', 'K',
        'l', 'L',
        'm', 'M',
        'n', 'N',
        'o', 'O',
        'p', 'P',
        'q', 'Q',
        'r', 'R',
        's', 'S',
        't', 'T',
        'u', 'U',
        'v', 'V',
        'w', 'W',
        'x', 'X',
        'y', 'Y',
        'z', 'Z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '!', '*', '%', '#', '@', ':', ';', '<', '>', ',', '.'
    };

    std::uniform_int_distribution<> distribution(0, (sizeof(AllowChars) / sizeof(*AllowChars)) - 1);

    std::string Result;
    for (size_t i = 0; i < 16; i++)
    {
        Result += AllowChars[distribution(generator)];
    }

    return Result;
}