// Dark Souls 3 - Open Server

#include "Server/Streams/Frpg2ReliableUdpFragmentStream.h"
#include "Server/Streams/Frpg2ReliableUdpFragment.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"
#include "Core/Utils/Compression.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Crypto/RSACipher.h"

Frpg2ReliableUdpFragmentStream::Frpg2ReliableUdpFragmentStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken)
    : Frpg2ReliableUdpPacketStream(Connection, CwcKey, AuthToken)
{
}

bool Frpg2ReliableUdpFragmentStream::Send(const Frpg2ReliableUdpFragment& Fragment)
{
    std::vector<uint8_t> Payload = Fragment.Payload;
    bool bCompressed = (Fragment.Payload.size() >= MIN_SIZE_FOR_COMPRESSION);
    uint32_t UncompressedSize = Payload.size();

    if (bCompressed)
    {        
        std::vector<uint8_t> UncompressPayload = Payload;
        if (!Compress(UncompressPayload, Payload))
        {
            Warning("[%s] Failed to compress packet data.", Connection->GetName().c_str());
            return false;
        }
    }

    int FragmentCount = (Fragment.Payload.size() + (MAX_FRAGMENT_LENGTH - 1)) / MAX_FRAGMENT_LENGTH;

    // Fragment up if payload is larger than max payload size.
    for (int i = 0; i < FragmentCount; i++)
    {
        int FragmentOffset = i * MAX_FRAGMENT_LENGTH;
        int BytesRemaining = Fragment.Payload.size() - FragmentOffset;
        int FragmentLength = std::min(MAX_FRAGMENT_LENGTH, BytesRemaining);

        Frpg2ReliableUdpFragment SendFragment;
        SendFragment.Header.compress_flag = bCompressed;
        SendFragment.Header.fragment_index = 0;
        SendFragment.Header.fragment_length = FragmentLength;
        SendFragment.Header.total_payload_length = Payload.size();
        SendFragment.Header.packet_counter = SentFragmentCounter++;
        SendFragment.PayloadDecompressedLength = UncompressedSize;
        SendFragment.Payload.resize(FragmentLength);

        memcpy(SendFragment.Payload.data(), Payload.data() + FragmentOffset, FragmentLength);

        Frpg2ReliableUdpPacket SendPacket;
        if (!EncodeFragment(SendFragment, SendPacket))
        {
            Warning("[%s] Failed to encode fragment to packet.", Connection->GetName().c_str());
            return false;
        }

        // TODO: Remove when we have a better way to handle this without breaking abstraction.
        // This sets the response ack counter which lets the reliable udp packet stream to send
        // this as a reply.
        if (i == 0)
        {
            SendPacket.Header.SetAckCounters(0, Fragment.AckSequenceIndex);
        }

        if (!Frpg2ReliableUdpPacketStream::Send(SendPacket))
        {
            Warning("[%s] Failed to send fragment packet.", Connection->GetName().c_str());
            return false;
        }
    }

    return false;
}

bool Frpg2ReliableUdpFragmentStream::Recieve(Frpg2ReliableUdpFragment* Fragment)
{
    if (RecieveQueue.size() > 0)
    {
        *Fragment = RecieveQueue[0];
        RecieveQueue.erase(RecieveQueue.begin());
        return true;
    }

    return false;
}

bool Frpg2ReliableUdpFragmentStream::RecieveInternal(Frpg2ReliableUdpFragment* Fragment)
{
    Frpg2ReliableUdpPacket Packet;
    if (!Frpg2ReliableUdpPacketStream::Recieve(&Packet))
    {
        return false;
    }

    if (!DecodeFragment(Packet, *Fragment))
    {
        Warning("[%s] Failed to convert packet payload to Fragment.", Connection->GetName().c_str());
        return false;
    }

    // TODO: Remove when we have a better way to handle this without breaking abstraction.
    uint32_t Remote;
    Packet.Header.GetAckCounters(Fragment->AckSequenceIndex, Remote);

    return true;
}

bool Frpg2ReliableUdpFragmentStream::DecodeFragment(const Frpg2ReliableUdpPacket& Packet, Frpg2ReliableUdpFragment& Fragment)
{
    if (Packet.Payload.size() < sizeof(Frpg2ReliableUdpFragmentHeader))
    {
        Warning("[%s] Packet payload is less than the minimum size of a Fragment, failed to deserialize.", Connection->GetName().c_str());
        return false;
    }

    memcpy(&Fragment.Header, Packet.Payload.data(), sizeof(Frpg2ReliableUdpFragmentHeader));
    Fragment.Header.SwapEndian();

    size_t PayloadOffset = sizeof(Frpg2ReliableUdpFragmentHeader);
    size_t PayloadLength = Packet.Payload.size() - PayloadOffset;

    if (Fragment.Header.compress_flag && Fragment.Header.fragment_index == 0)
    {
        memcpy(&Fragment.PayloadDecompressedLength, Packet.Payload.data() + PayloadOffset, 4);
        Fragment.PayloadDecompressedLength = BigEndianToHostOrder(Fragment.PayloadDecompressedLength);

        PayloadOffset += 4;
        PayloadLength -= 4;
    }

    Fragment.Payload.resize(PayloadLength);
    memcpy(Fragment.Payload.data(), Packet.Payload.data() + PayloadOffset, PayloadLength);

    return true;
}

bool Frpg2ReliableUdpFragmentStream::EncodeFragment(const Frpg2ReliableUdpFragment& Fragment, Frpg2ReliableUdpPacket& Packet)
{
    Frpg2ReliableUdpFragment ByteSwappedFragment = Fragment;
    ByteSwappedFragment.Header.SwapEndian();
    ByteSwappedFragment.PayloadDecompressedLength = HostOrderToBigEndian(ByteSwappedFragment.PayloadDecompressedLength);

    size_t PayloadSize = sizeof(Frpg2ReliableUdpFragment) + ByteSwappedFragment.Payload.size();
    if (ByteSwappedFragment.Header.compress_flag && ByteSwappedFragment.Header.fragment_index == 0)
    {
        PayloadSize += 4;
    }

    Packet.Payload.resize(PayloadSize);

    memcpy(Packet.Payload.data(), &ByteSwappedFragment.Header, sizeof(Frpg2ReliableUdpFragment));

    size_t WriteOffset = sizeof(Frpg2ReliableUdpFragmentHeader);
    if (ByteSwappedFragment.Header.compress_flag && ByteSwappedFragment.Header.fragment_index == 0)
    {
        memcpy(Packet.Payload.data() + WriteOffset, &ByteSwappedFragment.PayloadDecompressedLength, 4);
        WriteOffset += 4;
    }

    memcpy(Packet.Payload.data() + WriteOffset, ByteSwappedFragment.Payload.data(), ByteSwappedFragment.Payload.size());

    return true;
}

void Frpg2ReliableUdpFragmentStream::Reset()
{
    Frpg2ReliableUdpPacketStream::Reset();

    Fragments.clear();
    RecieveQueue.clear();
    RecievedFragmentLength = 0;
}

bool Frpg2ReliableUdpFragmentStream::Pump()
{
    if (Frpg2ReliableUdpPacketStream::Pump())
    {
        return true;
    }

    Frpg2ReliableUdpFragment Fragment;
    while (RecieveInternal(&Fragment))
    {
        RecievedFragmentLength += Fragment.Header.total_payload_length;
        if (RecievedFragmentLength >= Fragment.Header.fragment_length)
        {
            // Compact all payloads together into one combined packet.
            if (Fragments.size() > 0)
            {
                Frpg2ReliableUdpFragment CombinedFragment = Fragments[0];
                CombinedFragment.Header.fragment_index = 0;
                CombinedFragment.Header.fragment_length = Fragment.Header.total_payload_length;

                CombinedFragment.Payload.resize(CombinedFragment.Header.total_payload_length);
                int Offset = Fragments[0].Header.fragment_length;
                for (int i = 1; i < Fragments.size(); i++)
                {
                    const Frpg2ReliableUdpFragment& Fragment = Fragments[i];
                    memcpy(CombinedFragment.Payload.data() + Offset, Fragment.Payload.data(), Fragment.Payload.size());
                    Offset += Fragment.Payload.size();
                }

                memcpy(CombinedFragment.Payload.data() + Offset, Fragment.Payload.data(), Fragment.Payload.size());

                Fragment = CombinedFragment;
            }

            // Decompress data if required.
            if (Fragment.Header.compress_flag)
            {
                std::vector<uint8_t> UncompressPayload = Fragment.Payload;
                if (!Decompress(UncompressPayload, Fragment.Payload, Fragment.PayloadDecompressedLength))
                {
                    Warning("[%s] Failed to decompress packet data.", Connection->GetName().c_str());
                    return true;
                }

                Fragment.Header.compress_flag = false;
            }

            RecieveQueue.push_back(Fragment);
            RecievedFragmentLength = 0;
        }
    }

    return false;
}