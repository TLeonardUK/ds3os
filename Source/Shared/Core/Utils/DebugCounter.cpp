/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/DebugCounter.h"

DebugCounter::DebugCounter(const std::string& InName, double InRateWindow, double InRollingWindow)
    : Name(InName)
    , RollingWindow(InRollingWindow)
    , RateWindow(InRateWindow)
{
    Registry.push_back(this);
}

DebugCounter::~DebugCounter()
{
    if (auto Iter = std::find(Registry.begin(), Registry.end(), this); Iter != Registry.end())
    {
        Registry.erase(Iter);
    }
}

std::vector<DebugCounter*> DebugCounter::GetCounters()
{
    return Registry;
}

std::string DebugCounter::GetName()
{
    return Name;
}

double DebugCounter::GetAverageRate()
{
    std::scoped_lock lock(DataMutex);

    if (Samples.empty())
    {
        return 0.0;
    }

    double Total = 0.0f;

    for (Sample& Sample : Samples)
    {
        Total += Sample.Value;
    }

    double CurrentTime = GetHighResolutionSeconds();
    double ElapsedTime = CurrentTime - Samples.front().Time;
    double Multiplier = RateWindow / ElapsedTime;

    return Total * Multiplier;
}

double DebugCounter::GetTotalLifetime()
{
    return LifetimeTotal;
}

void DebugCounter::Add(double Value)
{
    std::scoped_lock lock(DataMutex);

    double CurrentTime = GetHighResolutionSeconds();

    Sample NewSample;
    NewSample.Time = CurrentTime;
    NewSample.Value = Value;
    Samples.push_back(NewSample);

    // Trim old samples from list.
    double OldestTime = CurrentTime - RollingWindow;
    while (!Samples.empty())
    {
        Sample& FirstSample = Samples.front();
        if (FirstSample.Time < OldestTime)
        {
            Samples.erase(Samples.begin());
        }
        else
        {
            break;
        }
    }

    LifetimeTotal += Value;
}