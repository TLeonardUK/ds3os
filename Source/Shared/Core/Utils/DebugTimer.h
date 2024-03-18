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

class DebugTimer 
{
public:
    DebugTimer(const std::string& name, double RollingWindow = 5.0);
    ~DebugTimer();

    std::string GetName();
    double GetAverage();
    double GetPeak();
    double GetCurrent();

    void Poll();

    static std::vector<DebugTimer*> GetTimers();
    static void PollAll();

protected:
    friend struct DebugTimerScope;

    void AddSample(double Interval);

private:    
    inline static std::vector<DebugTimer*> Registry;

    double RollingWindow;
    std::string Name;

    double Current = 0.0;
    double Peak = 0.0;
    double PeakTracker = 0.0;

    double Average = 0.0;
    double AverageTimer = 0.0;
    double AverageSum = 0.0;
    double AverageSamples = 0.0;

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