// Dark Souls 3 - Open Server

#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"
#include "Server/Streams/Frpg2ReliableUdpFragment.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

Frpg2ReliableUdpMessageStream::Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken)
    : Frpg2ReliableUdpFragmentStream(Connection, CwcKey, AuthToken)
{
}

bool Frpg2ReliableUdpMessageStream::SendInternal(const Frpg2ReliableUdpMessage& Message, const Frpg2ReliableUdpMessage* ResponseTo)
{
    Frpg2ReliableUdpMessage SendMessage;
    if (ResponseTo != nullptr)
    {
        SendMessage.Header.msg_index = ResponseTo->Header.msg_index;
        SendMessage.Header.msg_type = Frpg2ReliableUdpMessageType::Reply;
    }
    else
    {
        SendMessage.Header.msg_index = SentMessageCounter++;
    }

    Frpg2ReliableUdpFragment Packet;
    if (!EncodeMessage(SendMessage, Packet))
    {
        Warning("[%s] Failed to convert message to packet.", Connection->GetName().c_str());
        return false;
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
        if (ReliableUdpMessageType_Expects_Response(SendMessage.Header.msg_type))
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
        if (!Protobuf_To_ReliableUdpMessageType(Message, ResponseMessage.Header.msg_type))
        {
            Warning("[%s] Failed to determine message type by protobuf.", Connection->GetName().c_str());
            return false;
        }
    }
    else
    {
        // TODO: Remove when we have a better way to handle this without breaking abstraction.
        ResponseMessage.AckSequenceIndex = ResponseTo->AckSequenceIndex;
    }

    if (!Message->SerializeToArray(ResponseMessage.Payload.data(), ResponseMessage.Payload.size()))
    {
        Warning("[%s] Failed to serialize protobuf payload.", Connection->GetName().c_str());
        return false;
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
        Warning("[%s] Failed to convert packet payload to message.", Connection->GetName().c_str());
        return false;
    }

    // TODO: Remove when we have a better way to handle this without breaking abstraction.
    Message->AckSequenceIndex = Packet.AckSequenceIndex;

    // Create protobuf based on the type provided.
    Frpg2ReliableUdpMessageType MessageType = Message->Header.msg_type;
    bool IsResponse = false;
    if (Message->Header.msg_type == Frpg2ReliableUdpMessageType::Reply)
    {
        // Find message being replied to.
        if (auto iter = OutstandingResponses.find(Message->Header.msg_index); iter != OutstandingResponses.end())
        {
            MessageType = iter->second;
            OutstandingResponses.erase(iter);
        }
        else
        {
            Warning("[%s] Recieved unexpected response for message, dropping: type=0x%08x index=0x%08x", Connection->GetName().c_str(), MessageType, Message->Header.msg_index);
            return false;
        }

        IsResponse = true;
    }

    if (!ReliableUdpMessageType_To_Protobuf(MessageType, IsResponse, Message->Protobuf))
    {
        Warning("[%s] Failed to create protobuf instance for message: type=0x%08x index=0x%08x", Connection->GetName().c_str(), MessageType, Message->Header.msg_index);
        return false;
    }

    if (!Message->Protobuf->ParseFromArray(Message->Payload.data(), Message->Payload.size()))
    {
        Warning("[%s] Failed to deserialize protobuf instance for message: type=0x%08x index=0x%08x", Connection->GetName().c_str(), MessageType, Message->Header.msg_index);
        return true;
    }

    Log("[%s] Recieving message: type=0x%08x index=0x%08x", Connection->GetName().c_str(), Message->Header.msg_type, Message->Header.msg_index)

    return true;
}

bool Frpg2ReliableUdpMessageStream::DecodeMessage(const Frpg2ReliableUdpFragment& Packet, Frpg2ReliableUdpMessage& Message)
{
    if (Packet.Payload.size() < sizeof(Frpg2ReliableUdpMessageHeader))
    {
        Warning("[%s] Packet payload is less than the minimum size of a message, failed to deserialize.", Connection->GetName().c_str());
        return false;
    }

    memcpy(&Message.Header, Packet.Payload.data(), sizeof(Frpg2ReliableUdpMessageHeader));
    Message.Header.SwapEndian();

    size_t PayloadOffset = sizeof(Frpg2ReliableUdpMessageHeader);
    size_t PayloadLength = Packet.Payload.size() - PayloadOffset;

    Message.Payload.resize(PayloadLength);
    memcpy(Message.Payload.data(), Packet.Payload.data() + PayloadOffset, PayloadLength);

    return true;
}

bool Frpg2ReliableUdpMessageStream::EncodeMessage(const Frpg2ReliableUdpMessage& Message, Frpg2ReliableUdpFragment& Packet)
{
    Frpg2ReliableUdpMessage ByteSwappedMessage = Message;
    ByteSwappedMessage.Header.SwapEndian();

    size_t PayloadSize = sizeof(Frpg2ReliableUdpMessage) + ByteSwappedMessage.Payload.size();
    Packet.Payload.resize(PayloadSize);

    memcpy(Packet.Payload.data(), &ByteSwappedMessage.Header, sizeof(Frpg2ReliableUdpMessage));
    memcpy(Packet.Payload.data() + sizeof(Frpg2ReliableUdpMessageHeader), ByteSwappedMessage.Payload.data(), ByteSwappedMessage.Payload.size());

    return true;
}

void Frpg2ReliableUdpMessageStream::Reset()
{
    Frpg2ReliableUdpPacketStream::Reset();

    SentMessageCounter = 0;
    OutstandingResponses.clear();
}
