/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Utils/DebugCounter.h"

DebugCounter::DebugCounter(const std::string& InName)
    : Name(InName)
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
    return Average;
}

double DebugCounter::GetTotalLifetime()
{
    return LifetimeTotal;
}

void DebugCounter::Add(double Value)
{
    AverageSum += Value;
    LifetimeTotal += Value;
}

void DebugCounter::Poll()
{
    double CurrentTime = GetHighResolutionSeconds();
    double Elapsed = CurrentTime - AverageTimer;
    if (Elapsed > 1.0f)
    {
        float Scale = 1.0f / Elapsed;
        float Sample = AverageSum * Scale;

        Average = (Average * 0.9f) + (Sample * 0.1f);
        AverageSum = 0.0;
        AverageTimer = CurrentTime;
    }
}

void DebugCounter::PollAll()
{
    for (DebugCounter* instance : Registry)
    {
        instance->Poll();
    }
}