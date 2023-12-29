/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Shared/Core/Utils/Endian.h"
#include "Shared/Game/GameType.h"

#include <vector>
#include <memory>

#include "Protobuf/SharedProtobufs.h"

enum class DS2_Frpg2ReliableUdpMessageType : int
{
    Reply = 0x0,
    Push = 0x0320,

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         Type = OpCode,
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         Type = OpCode,
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Do Nothing */
#include "Server.DarkSouls2/Server/Streams/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE
};
