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
#include <variant>
#include <unordered_map>

#include "Shared/Platform/Platform.h"

// Simple class, you pipe in source/key/value pairs and it will write a log
// whenever the value of one changes. Useful for tracking when things are changing
// while playing the game.

class DiffTracker 
{
public:

    using ValueType = std::variant<size_t, std::string>;

    void Field(const std::string& Source, const std::string& Key, ValueType Value);

protected:
    struct SourceState
    {
        std::unordered_map<std::string, ValueType> Values;
    };

    bool AreValuesEqual(const ValueType& A, const ValueType& B);
    std::string GetValueString(const ValueType& A);

    std::shared_ptr<SourceState> FindOrAddSourceState(const std::string& Name);

private:    
    std::unordered_map<std::string, std::shared_ptr<SourceState>> Sources;
    
};
