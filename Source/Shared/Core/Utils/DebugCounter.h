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
#include <vector>
#include <list>

#include "Shared/Platform/Platform.h"

class DebugCounter
{
public:
    DebugCounter(const std::string& name);
    ~DebugCounter();

    std::string GetName();
    double GetAverageRate();
    double GetTotalLifetime();

    void Add(double Value);

    void Poll();

    static std::vector<DebugCounter*> GetCounters();
    static void PollAll();

private:
    inline static std::vector<DebugCounter*> Registry;    

    std::string Name;

    double LifetimeTotal = 0.0f;

    double Average = 0.0;
    double AverageTimer = 0.0;
    double AverageSum = 0.0;

};
