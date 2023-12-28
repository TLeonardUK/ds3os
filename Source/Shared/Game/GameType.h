/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

enum class GameType
{
    Unknown,

    DarkSouls2,
    DarkSouls3,

    COUNT
};

inline const char* GameTypeStrings[(int)GameType::COUNT] = {
    "Unknown",
    "DarkSouls2",
    "DarkSouls3",
};

bool ParseGameType(const char* text, GameType& Output);