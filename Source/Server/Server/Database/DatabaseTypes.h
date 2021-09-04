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

#include "Protobuf/Protobufs.h"

// Blood message stored in the database or live cache.
struct BloodMessage
{
    uint32_t MessageId;
    OnlineAreaId OnlineAreaId;

    uint32_t PlayerId;
    std::string PlayerSteamId;

    uint32_t CharacterId;

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

// Summon sign, only stored in live cache for now.
struct SummonSign
{
    uint32_t SignId;
    OnlineAreaId OnlineAreaId;

    uint32_t PlayerId;
    std::string PlayerSteamId;

    bool IsRedSign = false;

    Frpg2RequestMessage::MatchingParameter MatchingParameters;

    std::vector<uint8_t> PlayerStruct;

    uint32_t BeingSummonedByPlayerId = 0;
};

// Individual ranking in a leaderboard.
struct Ranking
{
    uint32_t Id;
    uint32_t BoardId;
    uint32_t PlayerId;
    uint32_t CharacterId;
    uint32_t Rank;
    uint32_t SerialRank;
    uint32_t Score;
    std::vector<uint8_t> Data;
};

// Individual character profile registered by a player.
struct Character
{
    uint32_t Id;
    uint32_t PlayerId;
    uint32_t CharacterId;
    std::vector<uint8_t> Data;
    
    uint32_t QuickMatchDuelRank = 0;
    uint32_t QuickMatchDuelXp = 0;
    uint32_t QuickMatchBrawlRank = 0;
    uint32_t QuickMatchBrawlXp = 0;
};