/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Utils/DiffTracker.h"
#include "Core/Utils/Logging.h"

std::shared_ptr<DiffTracker::SourceState> DiffTracker::FindOrAddSourceState(const std::string& Name)
{
    if (auto Iter = Sources.find(Name); Iter != Sources.end())
    {
        return Iter->second;
    }

    std::shared_ptr<SourceState> State = std::make_shared<SourceState>();
    Sources[Name] = State;

    return State;
}

bool DiffTracker::AreValuesEqual(const ValueType& A, const ValueType& B)
{
    if (A.index() != B.index())
    {
        return false;
    }

    if (std::holds_alternative<std::string>(A))
    {
        return std::get<std::string>(A) == std::get<std::string>(B);
    }
    else if (std::holds_alternative<size_t>(A))
    {
        return std::get<size_t>(A) == std::get<size_t>(B);
    }

    Ensure(false);
    return false;
}

std::string DiffTracker::GetValueString(const ValueType& A)
{
    if (std::holds_alternative<std::string>(A))
    {
        return std::get<std::string>(A);
    }
    else if (std::holds_alternative<size_t>(A))
    {
        return std::to_string(std::get<size_t>(A));
    }

    Ensure(false);
    return "unknown";
}

void DiffTracker::Field(const std::string& Source, const std::string& Key, ValueType Value)
{
    std::shared_ptr<SourceState> State = FindOrAddSourceState(Source);

    if (auto Iter = State->Values.find(Key); Iter != State->Values.end())
    {
        if (!AreValuesEqual(Iter->second, Value))
        {
            LogS(Source.c_str(), "-----------------> %s changed from %s to %s <-----------------", Key.c_str(), GetValueString(Iter->second).c_str(), GetValueString(Value).c_str());

            State->Values[Key] = Value;
        }
    }
    else
    {
        LogS(Source.c_str(), "-----------------> %s initial value %s <-----------------", Key.c_str(), GetValueString(Value).c_str());

        State->Values[Key] = Value;
    }
}
