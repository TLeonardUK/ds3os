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

#include "Protobuf/Protobufs.h"

#include "Server/GameService/Utils/GameIds.h"

// Represents the in-game state of a player. Each game client owns an instance of this class.

struct PlayerState
{
    // Steam ID of logged in user as a hex string.
    std::string SteamId = "";

    // Unique ID of the player account. Used for identifying the 
    // player in most packets past initial login, steam id is not used.
    uint32_t PlayerId = 0;

    // The name of the character the player is currently playing with. Will be empty
    // until the first RequestUpdatePlayerStatus is invoked.
    std::string CharacterName = "";

    // Current online matching area the player is in.
    OnlineAreaId CurrentArea = OnlineAreaId::None;

    // If the player is currently in a state they can be invaded in.
    bool IsInvadable = false;

    // Players current soul level.
    int SoulLevel = 0;

    // Players maximum weapon level. 
    int MaxWeaponLevel = 0;

    // What type of visitor the player can currently be summoned as.
    Frpg2RequestMessage::VisitorPool VisitorPool = Frpg2RequestMessage::VisitorPool::VisitorPool_None;

    // Information the player sends and periodically patches with 
    // RequestUpdatePlayerStatus requests.
    Frpg2PlayerData::AllStatus PlayerStatus;

};