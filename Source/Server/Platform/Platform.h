/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

// This file contains any platform specific function declarations.
// The actual definitions of the functions are in the platform 
// specific sub-folders.

#pragma once

#include "Core/Utils/Event.h"

// ========================================================================
// General platform setup functions.
// ========================================================================

// Initializes anything the platform requires globally (socket libraries etc).
bool PlatformInit();

// Cleans up anything that PlatformInit setup.
bool PlatformTerm();

// ========================================================================
// Abstract class contains an assortment of events that we bubble 
// up from the the platform we are running on.
// ========================================================================

class PlatformEvents 
{
public:
    ~PlatformEvents() = delete;

    // Broadcast when the user uses ctrl+c in the console (or one of the other
    // system control events occurs - shutdown/logoff/etc).

    using CtrlSignalDelegate = Delegate<>;
    using CtrlSignalEvent = Event<CtrlSignalDelegate>;

    static inline CtrlSignalEvent OnCtrlSignal;

}; // namespace PlatformEvents

// ========================================================================
// Debugging related platform functionality.
// ========================================================================

// Defines the color to use when writing to the console.
enum class ConsoleColor
{
    Red,
    Yellow,
    Green,
    White,
    Grey,

    Count
};

void WriteToConsole(ConsoleColor Color, const char* Message);

// ========================================================================
// Timing related functionality.
// ========================================================================

// Gets the time in seconds since the system started running.
// Be aware that this value is not guaranteed high-precision, don't
// use it for any realtime calculations.
double GetSeconds();

// This is similar to GetSeconds but uses a high resolution timer suitable for
// performance timings. There is an overhead for using this, so
// prefer use of GetSeconds where possible.
double GetHighResolutionSeconds();