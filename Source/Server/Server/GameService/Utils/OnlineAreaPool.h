/*
 * Dark Souls 3_Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/GameService/Utils/Gameids.h"

#include <unordered_map>
#include <queue>
#include <memory>

// Super simple cache split up spatially based on the online area.

template <typename ValueType>
struct OnlineAreaPool
{
public:
    using EntryId = uint32_t;

private:
    struct Area
    {
        std::unordered_map<EntryId, std::shared_ptr<ValueType>> Entries;
        std::queue<EntryId> RemoveOrderQueue;
    };

private:
    std::shared_ptr<Area> FindOrCreateArea(OnlineAreaId AreaId)
    {
        if (auto iter = AreaMap.find(AreaId); iter != AreaMap.end())
        {
            return iter->second;
        }

        std::shared_ptr<Area> NewArea = std::make_shared<Area>();
        AreaMap.insert({ AreaId, NewArea });
        return NewArea;
    }

public:
    bool Remove(OnlineAreaId AreaId, EntryId Id)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            AreaInstance->Entries.erase(iter);
            return true;
        }

        return false;
    }

    std::shared_ptr<ValueType> Find(OnlineAreaId AreaId, EntryId Id)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            return iter->second;
        }
        return nullptr;
    }

    bool Contains(OnlineAreaId AreaId, EntryId Id)
    {
        return Find(AreaId, Id) != nullptr;
    }

    bool Add(OnlineAreaId AreaId, EntryId Id, std::shared_ptr<ValueType> Value)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            return false;
        }

        AreaInstance->Entries.insert({ Id, Value });
        AreaInstance->RemoveOrderQueue.emplace(Id);
        TrimArea(AreaInstance);

        return true;
    }

    void Trim()
    {
        for (auto Pair : AreaMap)
        {
            std::shared_ptr<Area> AreaInstance = Pair->second();
            TrimArea(AreaInstance);
        }
    }

    void SetMaxEntriesPerArea(int Entries)
    {
        MaxEntriesPerArea = Entries;
    }

private:

    void TrimArea(std::shared_ptr<Area> AreaInstance)
    {
        while (AreaInstance->Entries.size() > 0 && AreaInstance->RemoveOrderQueue.size() > 0)
        {
            EntryId ToRemove = AreaInstance->RemoveOrderQueue.front();
            AreaInstance->RemoveOrderQueue.pop();

            if (auto iter = AreaInstance->Entries.find(ToRemove); iter != AreaInstance->Entries.end())
            {
                AreaInstance->Entries.erase(iter);
            }
        }
    }

private:
    std::unordered_map<OnlineAreaId, std::shared_ptr<Area>> AreaMap;
    int MaxEntriesPerArea  = 100;

};