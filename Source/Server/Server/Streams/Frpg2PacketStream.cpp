/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2PacketStream.h"

#include "Shared/Core/Network/NetConnection.h"

#include "Config/BuildConfig.h"

#include "Shared/Core/Utils/Endian.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"

#include "Shared/Core/Utils/File.h"

#include <cstring>

Frpg2PacketStream::Frpg2PacketStream(std::shared_ptr<NetConnection> InConnection)
    : Connection(InConnection)
{
    // Initial recieve state is the size of the header.
    PacketBytesRecieved = 0;
    RecievingPacketHeader = true;
    PacketBuffer.resize(sizeof(uint16_t));
}

bool Frpg2PacketStream::Pump()
{
    // If we have got into an error state (due to failed send/recieves) then 
    // we can bail now.
    if (InErrorState)
    {
        return true;
    }

    // Recieve any pending packets.
    if (IsRecieving)
    {
        while (true)
        {
            if (PacketBytesRecieved < PacketBuffer.size())
            {
                int BytesRecieved = 0;
                if (!Connection->Recieve(PacketBuffer, PacketBytesRecieved, (int)PacketBuffer.size() - PacketBytesRecieved, BytesRecieved))
                {
                    WarningS(Connection->GetName().c_str(), "Failed to recieve on connection.");
                    InErrorState = true;
                    return true;
                }

                if (BytesRecieved == 0)
                {
                    break;
                }

                PacketBytesRecieved += BytesRecieved;
            }

            if (PacketBytesRecieved >= PacketBuffer.size())
            {
                if (RecievingPacketHeader)
                {
                    uint16_t PacketLength = BigEndianToHostOrder(*reinterpret_cast<uint16_t*>(PacketBuffer.data()));
                    if (PacketLength == 0)
                    {
                        WarningS(Connection->GetName().c_str(), "Recieved packet length of 0, this is invalid.");
                        InErrorState = true;
                        return true;
                    }
                    if (PacketLength > BuildConfig::MAX_PACKET_LENGTH)
                    {
                        WarningS(Connection->GetName().c_str(), "Recieved packet length of %i, this is greater than the max packet length.", PacketLength);
                        InErrorState = true;
                        return true;
                    }

                    PacketBuffer.resize(PacketLength);
                }
                else
                {
                    Frpg2Packet Packet;
                    if (!BytesToPacket(PacketBuffer, Packet))
                    {
                        WarningS(Connection->GetName().c_str(), "Failed to parse recieved packet.");
                        InErrorState = true;
                        return true;
                    }

                    // Disassemble if required.
                    if constexpr (BuildConfig::DISASSEMBLE_RECIEVED_MESSAGES)
                    {
                        Packet.Disassembly = Disassemble(Packet);
                    }

                    RecieveQueue.push_back(Packet);

                    PacketBuffer.resize(sizeof(uint16_t));
                }

                PacketBytesRecieved = 0;
                RecievingPacketHeader = !RecievingPacketHeader;
            }
        }
    }

    return false;
}

bool Frpg2PacketStream::BytesToPacket(const std::vector<uint8_t>& Buffer, Frpg2Packet& Packet)
{    
    memcpy(reinterpret_cast<char*>(&Packet.Header), Buffer.data(), sizeof(Frpg2PacketHeader));
    Packet.Header.SwapEndian();

    if (Packet.Header.payload_length > BuildConfig::MAX_PACKET_LENGTH - sizeof(Frpg2PacketHeader) - sizeof(uint16_t))
    {
        WarningS(Connection->GetName().c_str(), "Packet payload length is greater than maximum packet length. Unable to deserialize.");
        InErrorState = true;
        return false;
    }

    Packet.Payload.resize(Packet.Header.payload_length);
    memcpy(Packet.Payload.data(), Buffer.data() + sizeof(Frpg2PacketHeader), Packet.Header.payload_length);

    return true;    
}

bool Frpg2PacketStream::PacketToBytes(const Frpg2Packet& Packet, std::vector<uint8_t>& Buffer)
{
    Frpg2PacketHeader ByteSwappedHeader = Packet.Header;
    ByteSwappedHeader.SwapEndian();

    if (Packet.Header.payload_length > BuildConfig::MAX_PACKET_LENGTH - sizeof(Frpg2PacketHeader) - sizeof(uint16_t))
    {
        WarningS(Connection->GetName().c_str(), "Packet payload length is greater than maximum packet length. Unable to serialize.");
        InErrorState = true;
        return false;
    }

    Buffer.resize(sizeof(Frpg2PacketHeader) + Packet.Payload.size());

    memcpy(Buffer.data(), reinterpret_cast<char*>(&ByteSwappedHeader), sizeof(Frpg2PacketHeader));
    memcpy(Buffer.data() + sizeof(Frpg2PacketHeader), Packet.Payload.data(), Packet.Payload.size());

    return true;
}

bool Frpg2PacketStream::Send(const Frpg2Packet& Packet)
{
    // Fill in the header of the packet.
    Frpg2Packet SendPacket = Packet;
    SendPacket.Header.send_counter = ++PacketsSent;
    SendPacket.Header.payload_length = (uint32_t)Packet.Payload.size();
    SendPacket.Header.payload_length_short = (uint16_t)Packet.Payload.size();

    // Disassemble if required.
    if constexpr (BuildConfig::DISASSEMBLE_SENT_MESSAGES)
    {
        SendPacket.Disassembly = Disassemble(SendPacket);
        SendPacket.Disassembly.append(Packet.Disassembly);

        Log("\n>> SENT\n%s", SendPacket.Disassembly.c_str());
    }

    std::vector<uint8_t> Bytes;
    if (!PacketToBytes(SendPacket, Bytes))
    {
        WarningS(Connection->GetName().c_str(), "Failed to send packet, unable to serialize.");
        InErrorState = true;
        return false;
    }

    std::vector<uint8_t> BytesWithHeader;
    BytesWithHeader.resize(Bytes.size() + sizeof(uint16_t));

    *reinterpret_cast<uint16_t*>(BytesWithHeader.data()) = HostOrderToBigEndian((uint16_t)Bytes.size());
    memcpy(BytesWithHeader.data() + sizeof(uint16_t), Bytes.data(), Bytes.size());

    if (!Connection->Send(BytesWithHeader, 0, (int)BytesWithHeader.size()))
    {
        WarningS(Connection->GetName().c_str(), "Failed to send packet,.");
        InErrorState = true;
        return false;
    }

    return true;
}

bool Frpg2PacketStream::Recieve(Frpg2Packet* OutputPacket)
{
    if (RecieveQueue.size() == 0)
    {
        return false;
    }

    *OutputPacket = RecieveQueue[0];
    RecieveQueue.erase(RecieveQueue.begin());

    return true;
}

std::string Frpg2PacketStream::Disassemble(const Frpg2Packet& Input)
{
    std::string Result = "";

    Result += "Packet:\n";
    Result += StringFormat("\t%-30s = %u\n", "send_counter", Input.Header.send_counter);
    Result += StringFormat("\t%-30s = %u\n", "unknown_1", Input.Header.unknown_1);
    Result += StringFormat("\t%-30s = %u\n", "unknown_2", Input.Header.unknown_2);
    Result += StringFormat("\t%-30s = %u\n", "payload_length", Input.Header.payload_length);
    Result += StringFormat("\t%-30s = %u\n", "unknown_3", Input.Header.unknown_3);
    Result += StringFormat("\t%-30s = %u\n", "payload_length_short", Input.Header.payload_length_short);

    return Result;
}