// Dark Souls 3 - Open Server

#pragma once

#include "Core/Utils/Endian.h"

#include <vector>
#include <memory>

#include "Protobuf/Frpg2RequestMessage.pb.h"

// All the id's of message type we can recieve.
enum class Frpg2ReliableUdpMessageType
{
    Reply = 0x0,

#define DEFINE_REQUEST_RESPONSE(OpCode, Type, ProtobufClass, ResponseProtobufClass)         Type = OpCode,
#define DEFINE_MESSAGE(OpCode, Type, ProtobufClass)                                         Type = OpCode,
#include "Server/Streams/Frpg2ReliableUdpMessageTypes.inc"
#undef DEFINE_MESSAGE
#undef DEFINE_REQUEST_RESPONSE
};

#pragma pack(push, 1)
struct Frpg2ReliableUdpMessageHeader
{
public:
    uint32_t                    header_size     = 0x0C;                                   
    Frpg2ReliableUdpMessageType msg_type        = Frpg2ReliableUdpMessageType::Reply;     // Will be 0 for message replies, the reponse type is derived by matching up the message_index to the request.
    uint32_t                    msg_index       = 0;                                      // Feels like flags. Seen 00 08 10 0A as values.

    void SwapEndian()
    {
        header_size = BigEndianToHostOrder(header_size);
        msg_type    = BigEndianToHostOrder(msg_type);
        msg_index   = LittleEndianToHostOrder(msg_index); // Message index remains in little endian for whatever reason.
    }
};
#pragma pack(pop)

static_assert(sizeof(Frpg2ReliableUdpMessageHeader) == 12, "Message header is not expected size.");

struct Frpg2ReliableUdpMessage
{
public:
    Frpg2ReliableUdpMessageHeader Header;

    // Gross: This breaks our abstraction, but I can't see any particularly nice
    // way of passing this around so we can send the appropriate DAT/DAT_ACK codes.
    uint32_t AckSequenceIndex = 0;

    std::shared_ptr<google::protobuf::MessageLite> Protobuf;

    std::vector<uint8_t> Payload;
};

bool Protobuf_To_ReliableUdpMessageType(google::protobuf::MessageLite* Message, Frpg2ReliableUdpMessageType& Output);
bool ReliableUdpMessageType_To_Protobuf(Frpg2ReliableUdpMessageType Type, bool IsResponse, std::shared_ptr<google::protobuf::MessageLite>& Output);
bool ReliableUdpMessageType_Expects_Response(Frpg2ReliableUdpMessageType Type);
