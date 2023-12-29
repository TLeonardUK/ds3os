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

#include "Protobuf/DS3_Protobufs.h"
#include "Server/GameService/Utils/DS3_GameIds.h"

#include "Server/GameService/PlayerState.h"

#define DEFINE_FIELD(type, name, default_value)                             \
    private: type name = default_value;                                     \
    public:                                                                 \
        const type& Get##name() const { return name; }                      \
        type& Get##name##_Mutable() { return name; }                        \
        void Set##name(const type& input) { name = input; Mutated(); }   

struct DS3_PlayerState : public PlayerState
{
public:

    // Current online matching area the player is in.
    DEFINE_FIELD(DS3_OnlineAreaId, CurrentArea, DS3_OnlineAreaId::None)
    
    // What type of visitor the player can currently be summoned as.
    DEFINE_FIELD(DS3_Frpg2RequestMessage::VisitorPool, VisitorPool, DS3_Frpg2RequestMessage::VisitorPool::VisitorPool_None)
    
    // Information the player sends and periodically patches with 
    // RequestUpdatePlayerStatus requests.
    DEFINE_FIELD(DS3_Frpg2PlayerData::AllStatus, PlayerStatus, DS3_Frpg2PlayerData::AllStatus())

    virtual uint32_t GetCurrentAreaId() override
    {
        return (uint32_t)CurrentArea;
    }

    virtual bool IsInGame() override
    {
        return GetPlayerStatus().has_player_status() &&
               GetPlayerStatus().has_play_data() &&
               GetPlayerId() != 0;
    }

    virtual size_t GetSoulCount() override
    {
        auto Status = GetPlayerStatus().player_status();
        return Status.souls();
    }

    virtual size_t GetSoulMemory() override
    {
        auto Status = GetPlayerStatus().player_status();
        return Status.soul_memory();
    }
    
    virtual size_t GetDeathCount() override
    {
        auto LogInfo = GetPlayerStatus().log_info();
        return LogInfo.death_count();;
    }
    
    virtual size_t GetMultiplayerSessionCount() override
    {
        auto LogInfo = GetPlayerStatus().log_info();
        return LogInfo.multiplay_count();
    }
    
    virtual double GetPlayTime() override
    {
        return GetPlayerStatus().play_data().play_time_seconds();
    }

    virtual std::string GetConvenantStatusDescription() override
    {
        auto Status = GetPlayerStatus().player_status();
        std::string CovenantState = GetEnumString<DS3_CovenantId>((DS3_CovenantId)Status.covenant());

        switch ((DS3_CovenantId)Status.covenant())
        {
        case DS3_CovenantId::Blade_of_the_Darkmoon:
        case DS3_CovenantId::Blue_Sentinels:
            {
                if (Status.has_can_summon_for_way_of_blue() && Status.can_summon_for_way_of_blue())
                {
                    CovenantState += " (Summonable)";
                }
                break;
            }
        case DS3_CovenantId::Watchdogs_of_Farron:
            {
                if (Status.has_can_summon_for_watchdog_of_farron() && Status.can_summon_for_watchdog_of_farron())
                {
                    CovenantState += " (Summonable)";
                }
                break;
            }
        case DS3_CovenantId::Aldrich_Faithfuls:
            {
                if (Status.has_can_summon_for_aldritch_faithful() && Status.can_summon_for_aldritch_faithful())
                {
                    CovenantState += " (Summonable)";
                }
                break;
            }
        case DS3_CovenantId::Spears_of_the_Church:
            {
                if (Status.has_can_summon_for_spear_of_church() && Status.can_summon_for_spear_of_church())
                {
                    CovenantState += " (Summonable)";
                }
                break;
            }
        }

        return CovenantState;
    }

    virtual std::string GetStatusDescription() override
    {
        auto Status = GetPlayerStatus().player_status();
        std::string Result = "";

        switch (Status.world_type())
        {
        case DS3_Frpg2PlayerData::WorldType::WorldType_None:
            {
                Result = "Loading or in menus";
                break;
            }
        case DS3_Frpg2PlayerData::WorldType::WorldType_Multiplayer:
            {
                if (Status.net_mode() == DS3_Frpg2PlayerData::NetMode::NetMode_None)
                {
                    Result = "Multiplayer alone";
                }
                else if (Status.net_mode() == DS3_Frpg2PlayerData::NetMode::NetMode_Host)
                {
                    DS3_InvasionTypeId TypeId = (DS3_InvasionTypeId)Status.invasion_type();
                    switch (TypeId)
                    {
                    case DS3_InvasionTypeId::Summon_White:
                    case DS3_InvasionTypeId::Summon_Red:
                    case DS3_InvasionTypeId::Summon_Purple_White:
                    case DS3_InvasionTypeId::Avatar:
                    case DS3_InvasionTypeId::Arena_Battle_Royal:
                    case DS3_InvasionTypeId::Umbasa_White:
                    case DS3_InvasionTypeId::Summon_Sunlight_White:
                    case DS3_InvasionTypeId::Summon_Sunlight_Dark:
                    case DS3_InvasionTypeId::Summon_Purple_Dark:
                    case DS3_InvasionTypeId::Covenant_Blade_of_the_Darkmoon:
                    case DS3_InvasionTypeId::Blue_Sentinel:
                    case DS3_InvasionTypeId::Force_Join_Session:
                    {
                        Result = "Hosting cooperation with " + GetEnumString<DS3_InvasionTypeId>(TypeId);
                        break;
                    }
                    case DS3_InvasionTypeId::Red_Hunter:
                    case DS3_InvasionTypeId::Invade_Red:
                    case DS3_InvasionTypeId::Covenant_Spear_of_the_Church:
                    case DS3_InvasionTypeId::Guardian_of_Rosaria:
                    case DS3_InvasionTypeId::Covenant_Watchdog_of_Farron:
                    case DS3_InvasionTypeId::Covenant_Aldrich_Faithful:
                    case DS3_InvasionTypeId::Invade_Sunlight_Dark:
                    case DS3_InvasionTypeId::Invade_Purple_Dark:
                    {
                        Result = "Being invaded by " + GetEnumString<DS3_InvasionTypeId>(TypeId);
                        break;
                    }
                    }
                }
                else if (Status.net_mode() == DS3_Frpg2PlayerData::NetMode::NetMode_Client)
                {
                    Result = "In another world";
                }
                break;
            }
        case DS3_Frpg2PlayerData::WorldType::WorldType_Singleplayer:
            {
                std::string status = "Playing solo";
                if (Status.is_invadable())
                {
                    status += " (Invadable)";
                }
                Result = status;
                break;
            }
        }

        return Result;
    }

};

#undef DEFINE_FIELD