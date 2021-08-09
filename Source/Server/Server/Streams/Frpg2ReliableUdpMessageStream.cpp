// Dark Souls 3 - Open Server

#include "Server/Streams/Frpg2ReliableUdpMessageStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Crypto/RSACipher.h"

Frpg2ReliableUdpMessageStream::Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken)
    : Frpg2ReliableUdpPacketStream(Connection, CwcKey, AuthToken)
{
}

bool Frpg2ReliableUdpMessageStream::Send(const Frpg2ReliableUdpMessage& Message)
{
    // TODO
    // TODO: Fragment
    // TODO: Compress

    return false;
}

bool Frpg2ReliableUdpMessageStream::Recieve(Frpg2ReliableUdpMessage* Message)
{
    // TODO

    return false;
}

bool Frpg2ReliableUdpMessageStream::RecieveInternal(Frpg2ReliableUdpMessage* Message)
{
    Frpg2ReliableUdpPacket Packet;
    if (!Frpg2ReliableUdpPacketStream::Recieve(&Packet))
    {
        return false;
    }

    if (!DecodeMessage(Packet, *Message))
    {
        Warning("[%s] Failed to convert packet payload to message.", Connection->GetName().c_str());
        return false;
    }

    return true;
}

bool Frpg2ReliableUdpMessageStream::DecodeMessage(const Frpg2ReliableUdpPacket& Packet, Frpg2ReliableUdpMessage& Message)
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

    if (Message.Header.compress_flag && Message.Header.fragment_index == 0)
    {
        memcpy(&Message.PayloadDecompressedLength, Packet.Payload.data() + PayloadOffset, 4);
        Message.PayloadDecompressedLength = BigEndianToHostOrder(Message.PayloadDecompressedLength);

        PayloadOffset += 4;
        PayloadLength -= 4;
    }

    memcpy(&Message.PayloadHeader, Packet.Payload.data() + PayloadOffset, sizeof(Frpg2ReliableUdpPayloadHeader));
    Message.PayloadHeader.SwapEndian();
    PayloadOffset += sizeof(Frpg2ReliableUdpPayloadHeader);
    PayloadLength -= sizeof(Frpg2ReliableUdpPayloadHeader);

    Message.Payload.resize(PayloadLength);
    memcpy(Message.Payload.data(), Packet.Payload.data() + PayloadOffset, PayloadLength);

    return true;
}

bool Frpg2ReliableUdpMessageStream::EncodeMessage(const Frpg2ReliableUdpMessage& Message, Frpg2ReliableUdpPacket& Packet)
{
    Frpg2ReliableUdpMessage ByteSwappedMessage = Message;
    ByteSwappedMessage.Header.SwapEndian();
    ByteSwappedMessage.PayloadHeader.SwapEndian();
    ByteSwappedMessage.PayloadDecompressedLength = HostOrderToBigEndian(ByteSwappedMessage.PayloadDecompressedLength);

    size_t PayloadSize = sizeof(Frpg2ReliableUdpMessage) + sizeof(Frpg2ReliableUdpPayloadHeader) + ByteSwappedMessage.Payload.size();
    if (ByteSwappedMessage.Header.compress_flag && ByteSwappedMessage.Header.fragment_index == 0)
    {
        PayloadSize += 4;
    }

    Packet.Payload.resize(PayloadSize);

    memcpy(Packet.Payload.data(), &ByteSwappedMessage.Header, sizeof(Frpg2ReliableUdpMessage));

    size_t WriteOffset = sizeof(Frpg2ReliableUdpMessageHeader);
    if (ByteSwappedMessage.Header.compress_flag && ByteSwappedMessage.Header.fragment_index == 0)
    {
        memcpy(Packet.Payload.data() + WriteOffset, &ByteSwappedMessage.PayloadDecompressedLength, 4);
        WriteOffset += 4;
    }

    memcpy(Packet.Payload.data() + WriteOffset, &ByteSwappedMessage.PayloadHeader, sizeof(Frpg2ReliableUdpPayloadHeader));
    WriteOffset += sizeof(Frpg2ReliableUdpPayloadHeader);

    memcpy(Packet.Payload.data() + WriteOffset, ByteSwappedMessage.Payload.data(), ByteSwappedMessage.Payload.size());

    return true;
}

void Frpg2ReliableUdpMessageStream::Reset()
{
    Frpg2ReliableUdpPacketStream::Reset();

    Fragments.clear();
    RecieveQueue.clear();
    RecievedFragmentLength = 0;
}

bool Frpg2ReliableUdpMessageStream::Pump()
{
    if (Frpg2ReliableUdpPacketStream::Pump())
    {
        return true;
    }

    Frpg2ReliableUdpMessage Message;
    while (RecieveInternal(&Message))
    {
        RecievedFragmentLength += Message.Header.total_payload_length;
        if (RecievedFragmentLength >= Message.Header.fragment_length)
        {
            // Compact all payloads together into one combined packet.
            if (Fragments.size() > 0)
            {
                Frpg2ReliableUdpMessage CombinedMessage = Fragments[0];
                CombinedMessage.Header.fragment_index = 0;
                CombinedMessage.Header.fragment_length = Message.Header.total_payload_length;

                CombinedMessage.Payload.resize(CombinedMessage.Header.total_payload_length);
                int Offset = Fragments[0].Header.fragment_length;
                for (int i = 1; i < Fragments.size(); i++)
                {
                    const Frpg2ReliableUdpMessage& Fragment = Fragments[i];
                    memcpy(CombinedMessage.Payload.data() + Offset, Fragment.Payload.data(), Fragment.Payload.size());
                    Offset += Fragment.Payload.size();
                }

                memcpy(CombinedMessage.Payload.data() + Offset, Message.Payload.data(), Message.Payload.size());

                Message = CombinedMessage;
            }

            // Decompress data if required.
            if (Message.Header.compress_flag)
            {
                Message.Header.compress_flag = false;

                // TODO
            }

            Log("[%s] Recieved Message: unknown_1=%i unknown_2=%i unknown_3=%i unknown_4=%i", Connection->GetName().c_str(),
                Message.PayloadHeader.packet_id,
                Message.PayloadHeader.unknown_2,
                Message.PayloadHeader.unknown_3,
                Message.PayloadHeader.unknown_4);

            // DEBUG DEBUG DEBUG
            /*
            std::vector<uint8_t> PayloadAndHeader;
            PayloadAndHeader.resize(Message.Payload.size() + sizeof(Frpg2ReliableUdpPayloadHeader));
            
            Frpg2ReliableUdpPayloadHeader SwappedHeader = Message.PayloadHeader;
            SwappedHeader.SwapEndian();
            memcpy(PayloadAndHeader.data(), &SwappedHeader, sizeof(Frpg2ReliableUdpPayloadHeader));
            memcpy(PayloadAndHeader.data() + sizeof(Frpg2ReliableUdpPayloadHeader), Message.Payload.data(), Message.Payload.size());

            static int Counter = 0;
            char filename[256];
            sprintf_s(filename, "Z:\\ds3os\\Protobuf\\Dumps\\RawCaptures\\recieve_%i.bin", Counter++);
            WriteBytesToFile(filename, PayloadAndHeader);
            */
            // DEBUG DEBUG DEBUG

            RecieveQueue.push_back(Message);
            RecievedFragmentLength = 0;
        }
    }

    return false;
}