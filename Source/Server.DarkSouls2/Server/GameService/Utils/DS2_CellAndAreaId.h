/*
 * Dark Souls 3_Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/Utils/DS2_GameIds.h"

struct DS2_CellAndAreaId
{
    uint64_t CellId;
    DS2_OnlineAreaId AreaId;

    bool operator==(const DS2_CellAndAreaId& other) const
    {
        return AreaId == other.AreaId &&
               CellId == other.CellId;
    }
};

template <>
struct std::hash<DS2_CellAndAreaId>
{
    std::size_t operator()(const DS2_CellAndAreaId& k) const
    {
        return std::hash<size_t>()((size_t)k.AreaId) ^ 
               (std::hash<size_t>()((size_t)k.CellId) << 1);
    }
};