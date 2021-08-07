// Dark Souls 3 - Open Server

#include "Server/Streams/Frpg2UdpPacketStream.h"
#include "Server/Streams/Frpg2UdpPacket.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"

#include "Core/Crypto/CWCCipher.h"

Frpg2UdpPacketStream::Frpg2UdpPacketStream(std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& InCwcKey, uint64_t InAuthToken)
    : Connection(InConnection)
    , CwcKey(InCwcKey)
    , AuthToken(InAuthToken)
{
    EncryptionCipher = std::make_shared<CWCCipher>(InCwcKey);
    DecryptionCipher = std::make_shared<CWCCipher>(InCwcKey);

    RecieveBuffer.resize(64 * 1024);
}

bool Frpg2UdpPacketStream::Pump()
{
    // If we have got into an error state (due to failed send/recieves) then 
    // we can bail now.
    if (InErrorState)
    {
        return true;
    }

    // Recieve any pending packets.
    while (true)
    {
        int BytesRecieved = 0;
        RecieveBuffer.resize(RecieveBuffer.capacity());
        if (!Connection->Recieve(RecieveBuffer, 0, RecieveBuffer.size(), BytesRecieved))
        {
            Warning("[%s] Failed to recieve on connection.", Connection->GetName().c_str());
            InErrorState = true;
            return true;
        }

        if (BytesRecieved > 0)
        {
            RecieveBuffer.resize(BytesRecieved);

            Frpg2UdpPacket Packet;
            if (!BytesToPacket(RecieveBuffer, Packet))
            {
                Warning("[%s] Failed to parse recieved packet.", Connection->GetName().c_str());
                InErrorState = true;
                return true;
            }

            if (Packet.Header.auth_token != AuthToken)
            {
                Warning("[%s] Recieved packet with missmatching auth token, expected 0x%016llx got 0x%016llx.", Connection->GetName().c_str(), AuthToken, Packet.Header.auth_token);
                InErrorState = true;
                return true;
            }

            if (DecryptionCipher)
            {
                std::vector<uint8_t> EncryptedBuffer = Packet.Payload;
                if (!DecryptionCipher->Decrypt(EncryptedBuffer, Packet.Payload))
                {
                    Warning("[%s] Failed to decrypt packet payload.", Connection->GetName().c_str());
                    InErrorState = true;
                    return false;
                }
            }

            RecieveQueue.push_back(Packet);
        }
        else
        {
            break;
        }
    }

    return false;
}

bool Frpg2UdpPacketStream::Send(const Frpg2UdpPacket& Packet)
{
    Frpg2UdpPacket SendPacket = Packet;

    std::vector<uint8_t> DecryptedBuffer = SendPacket.Payload;
    if (EncryptionCipher)
    {
        if (!EncryptionCipher->Encrypt(DecryptedBuffer, SendPacket.Payload))
        {
            Warning("[%s] Failed to encrypt packet payload.", Connection->GetName().c_str());
            InErrorState = true;
            return false;
        }
    }

    std::vector<uint8_t> Bytes;
    if (!PacketToBytes(SendPacket, Bytes))
    {
        Warning("[%s] Failed to send packet, unable to serialize.", Connection->GetName().c_str());
        InErrorState = true;
        return false;
    }

    if (!Connection->Send(Bytes, 0, Bytes.size()))
    {
        Warning("[%s] Failed to send packet,.", Connection->GetName().c_str());
        InErrorState = true;
        return false;
    }

    return true;
}

bool Frpg2UdpPacketStream::Recieve(Frpg2UdpPacket* OutputPacket)
{
    if (RecieveQueue.size() == 0)
    {
        return false;
    }

    *OutputPacket = RecieveQueue[0];
    RecieveQueue.erase(RecieveQueue.begin());

    return true;
}

bool Frpg2UdpPacketStream::BytesToPacket(const std::vector<uint8_t>& Buffer, Frpg2UdpPacket& Packet)
{
    int PayloadSize = Buffer.size() - sizeof(Packet.Header);

    memcpy(&Packet.Header, Buffer.data(), sizeof(Packet.Header));
    Packet.Payload.resize(PayloadSize);
    memcpy(Packet.Payload.data(), Buffer.data() + sizeof(Packet.Header), PayloadSize);

    return true;
}

bool Frpg2UdpPacketStream::PacketToBytes(const Frpg2UdpPacket& Packet, std::vector<uint8_t>& Buffer)
{
    Buffer.resize(sizeof(Packet.Header) + Packet.Payload.size());

    memcpy(Buffer.data(), &Packet.Header, sizeof(Packet.Header));
    memcpy(Buffer.data() + sizeof(Packet.Header), Packet.Payload.data(), Packet.Payload.size());

    return true;
}
