/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/DebugTimer.h"

#pragma optimize("", off)

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
    return Average;
}

double DebugTimer::GetPeak()
{
    return Peak;
}

double DebugTimer::GetCurrent()
{
    return Current;
}

std::vector<DebugTimer*> DebugTimer::GetTimers()
{
    return Registry;
}

void DebugTimer::AddSample(double Interval)
{
    AverageSum += Interval;
    AverageSamples += 1.0;
    if (Interval > PeakTracker)
    {
        PeakTracker = Interval;
    }
}

void DebugTimer::Poll()
{
    double CurrentTime = GetHighResolutionSeconds();
    double Elapsed = CurrentTime - AverageTimer;
    if (Elapsed > RollingWindow)
    {
        float Scale = RollingWindow / Elapsed;
        float Sample = (AverageSum / AverageSamples) * Scale;

        Current = Sample;
        Peak = PeakTracker;
        Average = (Average * 0.9f) + (Sample * 0.1f);
        AverageSum = 0.0;
        AverageSamples = 0.0;
        PeakTracker = 0.0;
        AverageTimer = CurrentTime;
    }
}

void DebugTimer::PollAll()
{
    for (DebugTimer* instance : Registry)
    {
        instance->Poll();
    }
}