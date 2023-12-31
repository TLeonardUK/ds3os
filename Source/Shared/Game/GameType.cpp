/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Game/GameType.h"

#include <memory>
#include <cstring>

bool ParseGameType(const char* text, GameType& Output)
{
    for (size_t i = 0; i < (int)GameType::COUNT; i++)
    {
        if (strcmp(text, GameTypeStrings[i]) == 0)
        {
            Output = (GameType)i;
            return true;
        }
    }

    return false;
}