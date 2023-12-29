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
#include "Server/GameService/Utils/DS3_GameIds.h"

class DS3_BloodMessageData
{
public:
    DS3_BloodMessageTemplate Template;
    DS3_BloodMessageWord Word;
    DS3_BloodMessageGesture Gesture;
    DS3_BloodMessageConjunction Conjunction;
    DS3_BloodMessageTemplate Second_Template;
    DS3_BloodMessageWord Second_Word;

public:
    std::string ToString();

public:
    static bool Parse(const std::vector<uint8_t>& data, DS3_BloodMessageData& output);

};