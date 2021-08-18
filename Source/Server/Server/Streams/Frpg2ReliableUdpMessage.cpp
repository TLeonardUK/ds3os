/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2ReliableUdpMessage.h"

bool Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass) \
    if (dynamic_cast<Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)      \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Type;                                 \
        return true;                                                                \
    }                                                                               
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                 \
    if (dynamic_cast<Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)      \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Type;                                 \
        return true;                                                                \
    }                                                                               
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                            \
    if (dynamic_cast<Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)      \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Push; /* Not using push */            \
        return true;                                                                \
    }    
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType InType, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)                     \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            Output = std::make_shared<Frpg2RequestMessage::ResponseProtobufClass>();                    \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<Frpg2RequestMessage::ProtobufClass>();                            \
        }                                                                                               \
        return true;                                                                                    \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                                     \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<Frpg2RequestMessage::ProtobufClass>();                            \
            return true;                                                                                \
        }                                                                                               \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                                /* Not supported on server, server only sends these */
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
    if (InType == Frpg2ReliableUdpMessageType::Push)                                        
    {                                                                                       
        return false;                                                                       
    }

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return false;                                                                       \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Not required, gets caught by Push test above */
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}
