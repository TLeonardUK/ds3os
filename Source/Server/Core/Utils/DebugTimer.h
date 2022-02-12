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

#include "Platform/Platform.h"

class DebugTimer 
{
public:
    DebugTimer(const std::string& name, double RollingWindow = 60.0);
    ~DebugTimer();

    std::string GetName();
    double GetAverage();
    double GetPeak();
    double GetCurrent();

    static std::vector<DebugTimer*> GetTimers();

protected:
    friend struct DebugTimerScope;

    void AddSample(double Interval);

private:    
    inline static std::vector<DebugTimer*> Registry;

    std::mutex DataMutex;

    struct Sample
    {
        double Time;
        double Value;
    };

    std::list<Sample> Samples;

    double RollingWindow;
    std::string Name;

};

struct DebugTimerScope 
{
public:
    DebugTimerScope(DebugTimer& InTimer)
    {
        StartTime = GetHighResolutionSeconds();
        Timer = &InTimer;
    }
    ~DebugTimerScope()
    {
        double ElapsedTime = GetHighResolutionSeconds() - StartTime;
        Timer->AddSample(ElapsedTime);
    }

private:
    double StartTime;
    DebugTimer* Timer;

};