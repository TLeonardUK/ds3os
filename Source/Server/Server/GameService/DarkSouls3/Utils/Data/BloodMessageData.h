/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <vector>
#include "Server/GameService/DarkSouls3/Utils/GameIds.h"

class BloodMessageData
{
public:
    BloodMessageTemplate Template;
    BloodMessageWord Word;
    BloodMessageGesture Gesture;
    BloodMessageConjunction Conjunction;
    BloodMessageTemplate Second_Template;
    BloodMessageWord Second_Word;

public:
    std::string ToString();

public:
    static bool Parse(const std::vector<uint8_t>& data, BloodMessageData& output);

};