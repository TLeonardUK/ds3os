/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2ReliableUdpMessage.h"

// ------------------------------------------------------------------------------------------------
//  Dark Souls 3
// ------------------------------------------------------------------------------------------------

bool DS3_Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass) \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS3_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                 \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS3_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                            \
    if (dynamic_cast<DS3_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Push; /* Not using push */            \
        return true;                                                                \
    }    
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS3_ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType InType, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)                     \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ResponseProtobufClass>();                    \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ProtobufClass>();                            \
        }                                                                                               \
        return true;                                                                                    \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                                     \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS3_Frpg2RequestMessage::ProtobufClass>();                            \
            return true;                                                                                \
        }                                                                                               \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                                /* Not supported on server, server only sends these */
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS3_ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Push)
    {                                                                                       
        return false;                                                                       
    }

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if ((int)InType == (int)DS3_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return false;                                                                       \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Not required, gets caught by Push test above */
#include "Server.DarkSouls3/Server/Streams/DarkSouls3/DS3_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

// ------------------------------------------------------------------------------------------------
//  Dark Souls 2
// ------------------------------------------------------------------------------------------------

bool DS2_Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass) \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS2_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                 \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = (Frpg2ReliableUdpMessageType)(int)DS2_Frpg2ReliableUdpMessageType::Type; \
        return true;                                                                \
    }                                                                               
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                            \
    if (dynamic_cast<DS2_Frpg2RequestMessage::ProtobufClass*>(Message) != nullptr)  \
    {                                                                               \
        Output = Frpg2ReliableUdpMessageType::Push; /* Not using push */            \
        return true;                                                                \
    }    
#include "Server.DarkSouls2/Server/Streams/DarkSouls2/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS2_ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType InType, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)                     \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ResponseProtobufClass>();                    \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ProtobufClass>();                            \
        }                                                                                               \
        return true;                                                                                    \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                                     \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                                      \
    {                                                                                                   \
        if (IsResponse)                                                                                 \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            Output = std::make_shared<DS2_Frpg2RequestMessage::ProtobufClass>();                            \
            return true;                                                                                \
        }                                                                                               \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                                /* Not supported on server, server only sends these */
#include "Server.DarkSouls2/Server/Streams/DarkSouls2/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool DS2_ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Push)
    {                                                                                       
        return false;                                                                       
    }

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if ((int)InType == (int)DS2_Frpg2ReliableUdpMessageType::Type)                                        \
    {                                                                                       \
        return false;                                                                       \
    }
#define DEFINE_PUSH_MESSAGE(OpCode, Type, ProtobufClass)                                    /* Not required, gets caught by Push test above */
#include "Server.DarkSouls2/Server/Streams/DarkSouls2/DS2_Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_PUSH_MESSAGE
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

// ------------------------------------------------------------------------------------------------
//  Generic
// ------------------------------------------------------------------------------------------------

bool Protobuf_To_ReliableUdpMessageType(GameType ServerGameType, google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output)
{
    switch (ServerGameType)
    {
        case GameType::DarkSouls3:
        {
            return DS3_Protobuf_To_ReliableUdpMessageType(Message, Output);
        }
        case GameType::DarkSouls2:
        {
            return DS2_Protobuf_To_ReliableUdpMessageType(Message, Output);
        }
    }
    return false;
}

bool ReliableUdpMessageType_To_Protobuf(GameType ServerGameType, Frpg2ReliableUdpMessageType Type, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output)
{
    switch (ServerGameType)
    {
        case GameType::DarkSouls3:
        {
            return DS3_ReliableUdpMessageType_To_Protobuf(Type, IsResponse, Output);
        }
        case GameType::DarkSouls2:
        {
            return DS3_ReliableUdpMessageType_To_Protobuf(Type, IsResponse, Output);
        }
    }
    return false;
}

bool ReliableUdpMessageType_Expects_Response(GameType ServerGameType, Frpg2ReliableUdpMessageType Type)
{
    switch (ServerGameType)
    {
        case GameType::DarkSouls3:
        {
            return DS3_ReliableUdpMessageType_Expects_Response(Type);
        }
        case GameType::DarkSouls2:
        {
            return DS3_ReliableUdpMessageType_Expects_Response(Type);
        }
    }
    return false;
}
