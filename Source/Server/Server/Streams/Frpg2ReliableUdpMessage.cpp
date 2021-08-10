// Dark Souls 3 - Open Server

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
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
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
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}

bool ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType InType)
{
#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                          \
    {                                                                                       \
        return true;                                                                        \
    }
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         \
    if (InType == Frpg2ReliableUdpMessageType::Type)                                          \
    {                                                                                       \
        return false;                                                                       \
    }
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE

    return false;
}
