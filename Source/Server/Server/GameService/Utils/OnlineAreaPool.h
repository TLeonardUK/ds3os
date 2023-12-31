/*
 * Dark Souls 3_Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <unordered_map>
#include <deque>
#include <numeric>
#include <memory>
#include <random>
#include <algorithm>
#include <iterator>
#include <vector>
#include <functional>

// Super simple cache split up spatially based on the online area.

template <typename IdType, typename ValueType>
struct OnlineAreaPool
{
public:
    using EntryId = uint32_t;

    using AreaFilterFunction_t = std::function<bool(IdType Id)>;

private:
    struct Area
    {
        IdType Id;
        std::unordered_map<EntryId, std::shared_ptr<ValueType>> Entries;
        std::deque<EntryId> RemoveOrderQueue;
    };

private:
    std::vector<std::shared_ptr<Area>> FindAreas(AreaFilterFunction_t Filter)
    {
        std::vector<std::shared_ptr<Area>> Result;

        for (auto& Pair : AreaMap)
        {
            if (Filter(Pair.first))
            {
                Result.push_back(Pair.second);
            }
        }

        return Result;
    }

    std::vector<std::shared_ptr<Area>> FindArea(AreaFilterFunction_t Filter)
    {
        std::vector<std::shared_ptr<Area>> Result;

        for (auto& Pair : AreaMap)
        {
            if (Filter(Pair.first))
            {
                return Pair.second;
            }
        }

        return nullptr;
    }

    std::shared_ptr<Area> FindOrCreateArea(IdType AreaId)
    {
        if (auto iter = AreaMap.find(AreaId); iter != AreaMap.end())
        {
            return iter->second;
        }

        std::shared_ptr<Area> NewArea = std::make_shared<Area>();
        NewArea->Id = AreaId;
        AreaMap.insert({ AreaId, NewArea });
        return NewArea;
    }

public:
    OnlineAreaPool()
        : RandomGenerator(RandomDevice())
    {
    }

    bool Remove(IdType AreaId, EntryId Id)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            std::shared_ptr<ValueType> Entry = iter->second;
            AreaInstance->Entries.erase(iter);
            
            // We can ignore the RemoveOrderQueue for now, it will get purged during Trim'ing
            // Saves iterating over entire deque.

            return true;
        }

        return false;
    }

    std::shared_ptr<ValueType> Find(IdType AreaId, EntryId Id)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            return iter->second;
        }
        return nullptr;
    }

    // Avoid using this one if you can, it has to look through a lot more areas.
    std::shared_ptr<ValueType> Find(EntryId Id)
    {
        for (auto AreaPair : AreaMap)
        {
            std::shared_ptr<Area> AreaInstance = AreaPair.second;
            if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
            {
                return iter->second;
            }
        }
        return nullptr;
    }

    size_t GetTotalEntries()
    {
        size_t Total = 0;
        for (auto AreaPair : AreaMap)
        {
            std::shared_ptr<Area> AreaInstance = AreaPair.second;
            Total += AreaInstance->Entries.size();
        }
        return Total;
    }

    std::vector<std::shared_ptr<ValueType>> GetRandomSet(IdType AreaId, int MaxCount)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);

        std::vector<std::shared_ptr<ValueType>> Result;

        // TODO: This is not efficient its O(n), do this better.

        // Get a list of entry id's
        std::vector<EntryId> EntryIds;
        EntryIds.reserve(AreaInstance->Entries.size());

        for (auto KeyPair : AreaInstance->Entries) 
        {
            EntryIds.push_back(KeyPair.first);
        }
        
        // Shuffle them up.
        std::shuffle(EntryIds.begin(), EntryIds.end(), RandomGenerator);

        // Select however many we need.
        int MaxToGather = std::min(MaxCount, (int)EntryIds.size());
        for (int i = 0; i < MaxToGather; i++)
        {
            Result.push_back(AreaInstance->Entries[EntryIds[i]]);
        }

        return Result;
    }

    std::vector<std::shared_ptr<ValueType>> GetRandomSet(int MaxCount, AreaFilterFunction_t Filter)
    {
        std::vector<std::shared_ptr<Area>> Areas = FindAreas(Filter);

        std::vector<std::shared_ptr<ValueType>> Result;

        int Remaining = MaxCount;
        for (auto& AreaInstance : Areas)
        {
            // Get a list of entry id's
            std::vector<EntryId> EntryIds;
            EntryIds.reserve(AreaInstance->Entries.size());

            for (auto KeyPair : AreaInstance->Entries)
            {
                EntryIds.push_back(KeyPair.first);
            }

            // Shuffle them up.
            std::shuffle(EntryIds.begin(), EntryIds.end(), RandomGenerator);

            // Select however many we need.
            int MaxToGather = std::min(Remaining, (int)EntryIds.size());
            for (int i = 0; i < MaxToGather; i++)
            {
                Result.push_back(AreaInstance->Entries[EntryIds[i]]);
                Remaining--;
            }
        }

        return Result;
    }

    std::vector<std::shared_ptr<ValueType>> GetRecentSet(IdType AreaId, int MaxCount, std::function<bool(std::shared_ptr<ValueType>)> FilterCallback)
    {
        std::vector<std::shared_ptr<ValueType>> Result;

        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);

        int RemainingCount = MaxCount;

        for (int i = 0; i < AreaInstance->RemoveOrderQueue.size() && RemainingCount > 0; i++)
        {
            EntryId Id = AreaInstance->RemoveOrderQueue[i];
            if (auto Iter = AreaInstance->Entries.find(Id); Iter != AreaInstance->Entries.end())
            {
                if (FilterCallback(Iter->second))
                {
                    Result.push_back(Iter->second);
                    RemainingCount--;
                }
            }
        }

        return Result;
    }

    bool Contains(IdType AreaId, EntryId Id)
    {
        return Find(AreaId, Id) != nullptr;
    }

    bool Add(IdType AreaId, EntryId Id, std::shared_ptr<ValueType> Value)
    {
        std::shared_ptr<Area> AreaInstance = FindOrCreateArea(AreaId);
        if (auto iter = AreaInstance->Entries.find(Id); iter != AreaInstance->Entries.end())
        {
            return false;
        }

        AreaInstance->Entries.insert({ Id, Value });
        AreaInstance->RemoveOrderQueue.push_back(Id);
        TrimArea(AreaInstance);

        return true;
    }

    void Trim()
    {
        for (auto Pair : AreaMap)
        {
            std::shared_ptr<Area> AreaInstance = Pair.second;
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
        while (AreaInstance->Entries.size() > MaxEntriesPerArea && AreaInstance->RemoveOrderQueue.size() > 0)
        {
            EntryId ToRemove = AreaInstance->RemoveOrderQueue.front();
            AreaInstance->RemoveOrderQueue.pop_front();

            if (auto iter = AreaInstance->Entries.find(ToRemove); iter != AreaInstance->Entries.end())
            {
                AreaInstance->Entries.erase(iter);
            }
        }
    }

private:
    std::unordered_map<IdType, std::shared_ptr<Area>> AreaMap;
    int MaxEntriesPerArea  = 100;

    std::random_device RandomDevice;
    std::mt19937 RandomGenerator;

};