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

#include "Platform/Platform.h"

class DebugCounter
{
public:
    DebugCounter(const std::string& name, double RateWindow = 60 * 60, double RollingWindow = 60 * 5);
    ~DebugCounter();

    std::string GetName();
    double GetAverageRate();
    double GetTotalLifetime();

    void Add(double Value);

    static std::vector<DebugCounter*> GetCounters();

private:
    inline static std::vector<DebugCounter*> Registry;    

    std::mutex DataMutex;

    struct Sample
    {
        double Time;
        double Value;
    };

    std::list<Sample> Samples;

    double LifetimeTotal = 0.0;
    double RateWindow = 0.0;
    double RollingWindow = 0.0;

    std::string Name;

};