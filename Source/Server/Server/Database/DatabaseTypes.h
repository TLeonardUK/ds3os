/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/Utils/GameIds.h"

// Blood message stored in the database or live cache.
struct BloodMessage
{
    uint32_t MessageId;
    OnlineAreaId OnlineAreaId;

    uint32_t PlayerId;
    std::string PlayerSteamId;

    uint32_t RatingPoor;
    uint32_t RatingGood;

    std::vector<uint8_t> Data;
};

// Bloodstain stored in the database or live cache.
struct Bloodstain
{
    uint32_t BloodstainId;
    OnlineAreaId OnlineAreaId;

    uint32_t PlayerId;
    std::string PlayerSteamId;

    std::vector<uint8_t> Data;
    std::vector<uint8_t> GhostData;
};

// Ghost stored in the database or live cache.
struct Ghost
{
    uint32_t GhostId;
    OnlineAreaId OnlineAreaId;

    uint32_t PlayerId;
    std::string PlayerSteamId;

    std::vector<uint8_t> Data;
};