/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2UdpPacketStream.h"
#include "Server/Streams/Frpg2UdpPacket.h"

#include "Shared/Core/Network/NetConnection.h"

#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/File.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Platform/Platform.h"

#include "Shared/Core/Crypto/CWCServerUDPCipher.h"
#include "Shared/Core/Crypto/CWCClientUDPCipher.h"

#include <thread>
#include <chrono>
#include <cstring>

Frpg2UdpPacketStream::Frpg2UdpPacketStream(std::shared_ptr<NetConnection> InConnection, const std::vector<uint8_t>& InCwcKey, uint64_t InAuthToken, bool AsClient)
    : Connection(InConnection)
    , CwcKey(InCwcKey)
    , AuthToken(InAuthToken)
    , IsClient(AsClient)
{
    if (AsClient)
    {
        EncryptionCipher = std::make_shared<CWCClientUDPCipher>(InCwcKey, AuthToken);
        DecryptionCipher = std::make_shared<CWCServerUDPCipher>(InCwcKey, AuthToken);
    }
    else
    {
        EncryptionCipher = std::make_shared<CWCServerUDPCipher>(InCwcKey, AuthToken);
        DecryptionCipher = std::make_shared<CWCClientUDPCipher>(InCwcKey, AuthToken);
    }

    RecieveBuffer.resize(64 * 1024);

    LastActivityTime = GetSeconds();
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
        if (!Connection->Recieve(RecieveBuffer, 0, (int)RecieveBuffer.size(), BytesRecieved))
        {
            WarningS(Connection->GetName().c_str(), "Failed to recieve on connection.");
            InErrorState = true;
            return true;
        }

        if (BytesRecieved > 0)
        {
            LastActivityTime = GetSeconds();

            RecieveBuffer.resize(BytesRecieved);

            Frpg2UdpPacket Packet;
            if (!BytesToPacket(RecieveBuffer, Packet))
            {
                WarningS(Connection->GetName().c_str(), "Failed to parse recieved packet.");
                InErrorState = true;
                return true;
            }

            if (DecryptionCipher)
            {        
                std::vector<uint8_t> EncryptedBuffer = Packet.Payload;
                if (!DecryptionCipher->Decrypt(EncryptedBuffer, Packet.Payload))
                {
                    WarningS(Connection->GetName().c_str(), "Failed to decrypt packet payload.");
                    InErrorState = true;
                    return false;
                }
            }

           /* static bool dumped = false;
            if (!dumped && Connection->IsConnected())
            {
                dumped = true;
                WriteBytesToFile("Z:\\ds3os\\Research\\Packet Traces\\game_login_compare\\from-game.dat", Packet.Payload);
            }*/

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
        if (SendPacket.HasConnectionPrefix)
        {
            dynamic_cast<CWCClientUDPCipher*>(EncryptionCipher.get())->SetPacketsHaveConnectionPrefix(true);
        }

        if (!EncryptionCipher->Encrypt(DecryptedBuffer, SendPacket.Payload))
        {
            WarningS(Connection->GetName().c_str(), "Failed to encrypt packet payload.");
            InErrorState = true;
            return false;
        }

        if (SendPacket.HasConnectionPrefix)
        {
            dynamic_cast<CWCClientUDPCipher*>(EncryptionCipher.get())->SetPacketsHaveConnectionPrefix(false);
        }
    }

    std::vector<uint8_t> Bytes;
    if (!PacketToBytes(SendPacket, Bytes))
    {
        WarningS(Connection->GetName().c_str(), "Failed to send packet, unable to serialize.");
        InErrorState = true;
        return false;
    }

    if (!Connection->Send(Bytes, 0, (int)Bytes.size()))
    {
        WarningS(Connection->GetName().c_str(), "Failed to send packet.");
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
    int PayloadSize = (int)Buffer.size();

    Packet.Payload.resize(PayloadSize);
    memcpy(Packet.Payload.data(), Buffer.data(), PayloadSize);

    return true;
}

bool Frpg2UdpPacketStream::PacketToBytes(const Frpg2UdpPacket& Packet, std::vector<uint8_t>& Buffer)
{
    Buffer.resize(Packet.Payload.size());

    memcpy(Buffer.data(), Packet.Payload.data(), Packet.Payload.size());

    return true;
}
