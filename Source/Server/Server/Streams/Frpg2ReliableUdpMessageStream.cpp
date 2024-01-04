/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpFragment.h"
#include "Server/Game.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Network/NetConnection.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/DebugObjects.h"
#include "Shared/Core/Utils/Protobuf.h"

#include "Protobuf/SharedProtobufs.h"

#include <google/protobuf/unknown_field_set.h>

Frpg2ReliableUdpMessageStream::Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken, bool AsClient, Game* InGameInterface)
    : Frpg2ReliableUdpFragmentStream(Connection, CwcKey, AuthToken, AsClient)
    , GameInterface(InGameInterface)
{
}

bool Frpg2ReliableUdpMessageStream::SendInternal(const Frpg2ReliableUdpMessage& Message, const Frpg2ReliableUdpMessage* ResponseTo)
{
    Frpg2ReliableUdpMessage SendMessage = Message;
    if (SendMessage.Header.IsType(Frpg2ReliableUdpMessageType::Push))
    {
        SendMessage.Header.msg_index = 0xFFFFFFFF;

        Debug::PushMessagesSent.Add(1);
    }
    else if (ResponseTo != nullptr)
    {
        SendMessage.Header.msg_index = ResponseTo->Header.msg_index;
        SendMessage.Header.msg_type = Frpg2ReliableUdpMessageType::Reply;

        Debug::ResponsesSent.Add(1);
    }
    else
    {
        SendMessage.Header.msg_index = SentMessageCounter++;
        LastSentMessageIndex = SendMessage.Header.msg_index;

        Debug::ResponsesSent.Add(1);
    }

    Frpg2ReliableUdpFragment Packet;
    if (!EncodeMessage(SendMessage, Packet))
    {
        WarningS(Connection->GetName().c_str(), "Failed to convert message to packet.");
        InErrorState = true;
        return false;
    }

    // Disassemble if required.
    if constexpr (BuildConfig::DISASSEMBLE_SENT_MESSAGES)
    {
        Packet.Disassembly = Disassemble(SendMessage);
    }

    // TODO: Remove when we have a better way to handle this without breaking abstraction.
    if (ResponseTo != nullptr)
    {
        Packet.AckSequenceIndex = ResponseTo->AckSequenceIndex;
    }

    if (!Frpg2ReliableUdpFragmentStream::Send(Packet))
    {
        return false;
    }

    if (SendMessage.Header.msg_type != Frpg2ReliableUdpMessageType::Reply)
    {
        if (GameInterface->ReliableUdpMessageType_Expects_Response(SendMessage.Header.msg_type))
        {
            OutstandingResponses.insert({ SendMessage.Header.msg_index, SendMessage.Header.msg_type });
        }
    }

    return true;
}

bool Frpg2ReliableUdpMessageStream::Send(google::protobuf::MessageLite* Message, const Frpg2ReliableUdpMessage* ResponseTo)
{
    Frpg2ReliableUdpMessage ResponseMessage;
    ResponseMessage.Payload.resize(Message->ByteSize());

    if (ResponseTo == nullptr)
    {
        if (!GameInterface->Protobuf_To_ReliableUdpMessageType(Message, ResponseMessage.Header.msg_type))
        {
            WarningS(Connection->GetName().c_str(), "Failed to determine message type by protobuf.");
            InErrorState = true;
            return false;
        }
    }
    else
    {
        // TODO: Remove when we have a better way to handle this without breaking abstraction.
        ResponseMessage.AckSequenceIndex = ResponseTo->AckSequenceIndex;
    }

    if (!Message->SerializeToArray(ResponseMessage.Payload.data(), (int)ResponseMessage.Payload.size()))
    {
        WarningS(Connection->GetName().c_str(), "Failed to serialize protobuf payload.");
        InErrorState = true;
        return false;
    }

    if (!SendInternal(ResponseMessage, ResponseTo))
    {
        return true;
    }

    if constexpr (BuildConfig::LOG_PROTOBUF_STREAM)
    {
        Log(">> %s", Message->GetTypeName().c_str());
    }

    return true;
}

bool Frpg2ReliableUdpMessageStream::SendRawProtobuf(const std::vector<uint8_t>& Data, const Frpg2ReliableUdpMessage* ResponseTo)
{
    Frpg2ReliableUdpMessage ResponseMessage;
    ResponseMessage.Payload = Data;

    if (ResponseTo == nullptr)
    {
        ResponseMessage.Header.msg_type = Frpg2ReliableUdpMessageType::Push;
    }
    else
    {
        // TODO: Remove when we have a better way to handle this without breaking abstraction.
        ResponseMessage.AckSequenceIndex = ResponseTo->AckSequenceIndex;
    }

    if (!SendInternal(ResponseMessage, ResponseTo))
    {
        return true;
    }

    return true;
}

bool Frpg2ReliableUdpMessageStream::Recieve(Frpg2ReliableUdpMessage* Message)
{
    Frpg2ReliableUdpFragment Packet;
    if (!Frpg2ReliableUdpFragmentStream::Recieve(&Packet))
    {
        return false;
    }

    if (!DecodeMessage(Packet, *Message))
    {
        WarningS(Connection->GetName().c_str(), "Failed to convert packet payload to message.");
        InErrorState = true;
        return false;
    }

    Debug::RequestsRecieved.Add(1);

    // Disassemble if required.
    if constexpr (BuildConfig::DISASSEMBLE_RECIEVED_MESSAGES)
    {
        Message->Disassembly = Packet.Disassembly;
        Message->Disassembly.append(Disassemble(*Message));

        Log("\n<< RECV\n%s", Message->Disassembly.c_str());
    }

    // TODO: Remove when we have a better way to handle this without breaking abstraction.
    Message->AckSequenceIndex = Packet.AckSequenceIndex;

    // Create protobuf based on the type provided.
    Frpg2ReliableUdpMessageType MessageType = Message->Header.msg_type;
    bool IsResponse = false;
    if (Message->Header.IsType(Frpg2ReliableUdpMessageType::Reply))
    {
        // Find message being replied to.
        if (auto iter = OutstandingResponses.find(Message->Header.msg_index); iter != OutstandingResponses.end())
        {
            MessageType = iter->second;
            OutstandingResponses.erase(iter);
        }
        else
        {
            WarningS(Connection->GetName().c_str(), "Recieved unexpected response for message, dropping: type=0x%08x index=0x%08x", MessageType, Message->Header.msg_index);
            InErrorState = true;
            return false;
        }

        IsResponse = true;
    }

    if (!GameInterface->ReliableUdpMessageType_To_Protobuf(MessageType, IsResponse, Message->Protobuf))
    {
        WarningS(Connection->GetName().c_str(), "Failed to create protobuf instance for message: type=0x%08x index=0x%08x", MessageType, Message->Header.msg_index);

        if constexpr (BuildConfig::DUMP_FAILED_DISASSEMBLED_PACKETS)
        {            
            std::filesystem::path file_path = StringFormat("Debug\\Packets\\No Handler\\0x%04x\\%i.bin", (int)MessageType, DumpMessageIndex++);
            std::filesystem::create_directories(file_path.parent_path());
            WriteBytesToFile(file_path, Message->Payload);
        }

        InErrorState = true;
        return false;
    }

    if (!Message->Protobuf->ParseFromArray(Message->Payload.data(), (int)Message->Payload.size()))
    {
        WarningS(Connection->GetName().c_str(), "Failed to deserialize protobuf instance for message: type=0x%08x index=0x%08x", MessageType, Message->Header.msg_index);

        DecodedProtobufRegistry registry;
        registry.Decode("FailedMessage", Message->Payload.data(), (int)Message->Payload.size());
        WarningS(Connection->GetName().c_str(), "Decoded:\n%s", registry.ToString().c_str());

        if constexpr (BuildConfig::DUMP_FAILED_DISASSEMBLED_PACKETS)
        {
            std::filesystem::path file_path = StringFormat("Debug\\Packets\\Failed Deserialize\\0x%04x\\%i.bin", (int)MessageType, DumpMessageIndex++);
            std::filesystem::create_directories(file_path.parent_path());
            WriteBytesToFile(file_path, Message->Payload);
        }

        InErrorState = true;
        return false;
    }

    if constexpr (BuildConfig::LOG_PROTOBUF_STREAM)
    {
        Log("<< %s", Message->Protobuf->GetTypeName().c_str());
    }

    return true;
}

bool Frpg2ReliableUdpMessageStream::DecodeMessage(const Frpg2ReliableUdpFragment& Packet, Frpg2ReliableUdpMessage& Message)
{
    if (Packet.Payload.size() < sizeof(Frpg2ReliableUdpMessageHeader))
    {
        WarningS(Connection->GetName().c_str(), "Packet payload is less than the minimum size of a message, failed to deserialize.");
        InErrorState = true;
        return false;
    }

    int ReadOffset = 0;
    memcpy(&Message.Header, Packet.Payload.data(), sizeof(Frpg2ReliableUdpMessageHeader));
    ReadOffset += sizeof(Frpg2ReliableUdpMessageHeader);
    Message.Header.SwapEndian();

    if (Message.Header.IsType(Frpg2ReliableUdpMessageType::Reply))
    {
        memcpy(&Message.ResponseHeader, Packet.Payload.data() + ReadOffset, sizeof(Frpg2ReliableUdpMessageResponseHeader));
        ReadOffset += sizeof(Frpg2ReliableUdpMessageResponseHeader);
    }

    size_t PayloadLength = Packet.Payload.size() - ReadOffset;

    Message.Payload.resize(PayloadLength);
    memcpy(Message.Payload.data(), Packet.Payload.data() + ReadOffset, PayloadLength);

    return true;
}

bool Frpg2ReliableUdpMessageStream::EncodeMessage(const Frpg2ReliableUdpMessage& Message, Frpg2ReliableUdpFragment& Packet)
{
    Frpg2ReliableUdpMessage ByteSwappedMessage = Message;
    ByteSwappedMessage.Header.SwapEndian();
    ByteSwappedMessage.ResponseHeader.SwapEndian();

    size_t PayloadSize = sizeof(Frpg2ReliableUdpMessageHeader) + ByteSwappedMessage.Payload.size();
    if (Message.Header.IsType(Frpg2ReliableUdpMessageType::Reply))
    {
        PayloadSize += sizeof(Frpg2ReliableUdpMessageResponseHeader);
    }

    Packet.Payload.resize(PayloadSize);

    int WriteOffset = 0;
    memcpy(Packet.Payload.data() + WriteOffset, &ByteSwappedMessage.Header, sizeof(Frpg2ReliableUdpMessageHeader));
    WriteOffset += sizeof(Frpg2ReliableUdpMessageHeader);

    if (Message.Header.IsType(Frpg2ReliableUdpMessageType::Reply))
    {
        memcpy(Packet.Payload.data() + WriteOffset, &ByteSwappedMessage.ResponseHeader, sizeof(Frpg2ReliableUdpMessageResponseHeader));
        WriteOffset += sizeof(Frpg2ReliableUdpMessageResponseHeader);
    }

    memcpy(Packet.Payload.data() + WriteOffset, ByteSwappedMessage.Payload.data(), ByteSwappedMessage.Payload.size());

    return true;
}

void Frpg2ReliableUdpMessageStream::Reset()
{
    Frpg2ReliableUdpPacketStream::Reset();

    SentMessageCounter = 0;
    OutstandingResponses.clear();
}

std::string Frpg2ReliableUdpMessageStream::Disassemble(const Frpg2ReliableUdpMessage& Message)
{
    std::string Result = "";

    Result += "Message:\n";
    Result += StringFormat("\t%-30s = %u\n", "header_size", Message.Header.header_size);
    Result += StringFormat("\t%-30s = %u\n", "msg_type", Message.Header.msg_type);
    Result += StringFormat("\t%-30s = %u\n", "msg_index", Message.Header.msg_index);

    if (Message.Header.IsType(Frpg2ReliableUdpMessageType::Reply))
    {
        Result += StringFormat("\t%-30s = %u\n", "unknown_1", Message.ResponseHeader.unknown_1);
        Result += StringFormat("\t%-30s = %u\n", "unknown_2", Message.ResponseHeader.unknown_2);
        Result += StringFormat("\t%-30s = %u\n", "unknown_3", Message.ResponseHeader.unknown_3);
        Result += StringFormat("\t%-30s = %u\n", "unknown_4", Message.ResponseHeader.unknown_4);
    }

    Result += "Message Payload:\n";
    Result += BytesToString(Message.Payload, "\t");

    return Result;    
}