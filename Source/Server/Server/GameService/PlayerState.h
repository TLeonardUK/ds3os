/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <string>

// Represents the in-game state of a player. Each game client owns an instance of this class.

struct PlayerState
{
    // Steam ID of logged in user as a hex string.
    std::string SteamId;

    // Unique ID of the player account. Used for identifying the 
    // player in most packets past initial login, steam id is not used.
    uint32_t PlayerId;

    // The name of the character the player is currently playing with. Will be empty
    // until the first RequestUpdatePlayerStatus is invoked.
    std::string CharacterName;
};