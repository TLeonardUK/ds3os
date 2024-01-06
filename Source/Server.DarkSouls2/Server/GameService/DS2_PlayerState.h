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

#include "Protobuf/DS2_Protobufs.h"
#include "Server/GameService/Utils/DS2_GameIds.h"

#include "Server/GameService/PlayerState.h"

#define DEFINE_FIELD(type, name, default_value)                             \
    private: type name = default_value;                                     \
    public:                                                                 \
        const type& Get##name() const { return name; }                      \
        type& Get##name##_Mutable() { return name; }                        \
        void Set##name(const type& input) { name = input; Mutated(); }   

struct DS2_PlayerState : public PlayerState
{
public:

    // Current online matching area the player is in.
    DEFINE_FIELD(DS2_OnlineAreaId, CurrentArea, DS2_OnlineAreaId::None)
    
    // Similar to currentarea but more finely set, will be set to 0 if no online
    // activity can happen in the area.
    DEFINE_FIELD(int, CurrentOnlineActivityArea, 0)    
    
    // What type of visitor the player can currently be summoned as.
    DEFINE_FIELD(DS2_Frpg2RequestMessage::VisitorType, VisitorPool, DS2_Frpg2RequestMessage::VisitorType::VisitorType_None)
    
    // Information the player sends and periodically patches with 
    // RequestUpdatePlayerStatus requests.
    DEFINE_FIELD(DS2_Frpg2PlayerData::AllStatus, PlayerStatus, DS2_Frpg2PlayerData::AllStatus())

    virtual uint32_t GetCurrentAreaId() override
    {
        return (uint32_t)CurrentArea;
    }

    virtual bool IsInGame() override
    {
        return GetPlayerStatus().has_player_status() &&
               GetPlayerId() != 0;
    }

    virtual size_t GetSoulCount() override
    {
        return 0;
    }

    virtual size_t GetSoulMemory() override
    {
        auto Status = GetPlayerStatus().player_status();
        return Status.soul_memory();
    }

    virtual size_t GetDeathCount() override
    {
        return 0;
    }

    virtual size_t GetMultiplayerSessionCount() override
    {
        return 0;
    }

    virtual double GetPlayTime() override
    {
        auto Status = GetPlayerStatus().player_status();
        return Status.play_time_seconds();
    }

    virtual std::string GetConvenantStatusDescription() override
    {
        return "";
    }

    virtual std::string GetStatusDescription() override
    {
        return "";
    }

};

#undef DEFINE_FIELD