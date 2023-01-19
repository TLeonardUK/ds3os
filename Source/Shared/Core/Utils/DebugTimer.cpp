/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/DebugTimer.h"

DebugTimer::DebugTimer(const std::string& InName, double InRollingWindow)
    : Name(InName)
    , RollingWindow(InRollingWindow)
{
    Registry.push_back(this);
}

DebugTimer::~DebugTimer()
{
    if (auto Iter = std::find(Registry.begin(), Registry.end(), this); Iter != Registry.end())
    {
        Registry.erase(Iter);
    }
}

std::string DebugTimer::GetName()
{
    return Name;
}

double DebugTimer::GetAverage()
{
    std::scoped_lock lock(DataMutex);

    if (Samples.empty())
    {
        return 0.0;
    }

    double Average = 0.0f;

    for (Sample& Sample : Samples)
    {
        Average += Sample.Value;
    }

    return Average / Samples.size();
}

double DebugTimer::GetPeak()
{
    std::scoped_lock lock(DataMutex);

    double Peak = 0.0f;

    for (Sample& Sample : Samples)
    {
        if (Sample.Value > Peak)
        {
            Peak = Sample.Value;
        }
    }

    return Peak;
}

double DebugTimer::GetCurrent()
{
    std::scoped_lock lock(DataMutex);

    if (Samples.empty())
    {
        return 0.0;
    }

    return Samples.back().Value;
}

std::vector<DebugTimer*> DebugTimer::GetTimers()
{
    return Registry;
}

void DebugTimer::AddSample(double Interval)
{
    std::scoped_lock lock(DataMutex);

    double CurrentTime = GetHighResolutionSeconds();

    Sample NewSample;
    NewSample.Time = CurrentTime;
    NewSample.Value = Interval;
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
}