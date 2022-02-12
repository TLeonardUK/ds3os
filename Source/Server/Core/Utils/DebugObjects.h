/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Utils/DebugCounter.h"
#include "Core/Utils/DebugTimer.h"

namespace Debug
{
#define TIMER(Name, Description) extern DebugTimer Name;
#define COUNTER(Name, Description) extern DebugCounter Name;

#include "Core/Utils/DebugObjects.inc"

#undef TIMER
#undef COUNTER
}; 
