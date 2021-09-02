/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Network/NetIPAddress.h"

// Blocking call that gets the local machines external IPv4 address.
bool GetMachineIPv4(NetIPAddress& Output, bool GetPublicAddress = true);