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

#include "Server/GameService/PlayerState.h"

struct DS2_PlayerState : public PlayerState
{
public:

    virtual uint32_t GetCurrentAreaId() override
    {
        return 0;
    }

    virtual bool IsInGame() override
    {
        return false;
    }

    virtual size_t GetSoulCount() override
    {
        return 0;
    }

    virtual size_t GetSoulMemory() override
    {
        return 0;
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
        return 0.0;
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