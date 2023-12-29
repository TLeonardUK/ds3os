/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Streams/Frpg2ReliableUdpFragmentStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"

#include "Protobuf/SharedProtobufs.h"

#include <unordered_map>

class Cipher;
class Game;

class Frpg2ReliableUdpMessageStream
    : public Frpg2ReliableUdpFragmentStream
{
public:
    Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken, bool AsClient, Game* InGameInterface);

    // Short hand version of Send for protobufs, takes care of constructing the wrapper message.
    virtual bool Send(google::protobuf::MessageLite* Message, const Frpg2ReliableUdpMessage* ResponseTo = nullptr);

    // If we have a protobuf thats already serialized we can send it via this. Code assumes it should be sent with Push message type.
    virtual bool SendRawProtobuf(const std::vector<uint8_t>& Data, const Frpg2ReliableUdpMessage* ResponseTo = nullptr);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpMessage* Message);

    // This is kinda gross, we shouldn't expose this we should wrap it in a nice interface.
    // This returns the ack sequence number of the last sent message. Higher level code
    // can use this to manually wait for responses to specific messages.
    uint32_t GetLastSentMessageIndex() { return LastSentMessageIndex; }

    // Diassembles a messages into a human-readable string.
    std::string Disassemble(const Frpg2ReliableUdpMessage& Message);

protected:

    // Returns true if send was successful, if false is returned the send queue
    // is likely saturated or the packet is invalid.
    virtual bool SendInternal(const Frpg2ReliableUdpMessage& Message, const Frpg2ReliableUdpMessage* ResponseTo = nullptr);

    bool DecodeMessage(const Frpg2ReliableUdpFragment& Packet, Frpg2ReliableUdpMessage& Message);
    bool EncodeMessage(const Frpg2ReliableUdpMessage& Message, Frpg2ReliableUdpFragment& Packet);

    virtual void Reset() override;

private:

    struct MessageHistoryEntry
    {
        uint32_t MessageIndex;
        Frpg2ReliableUdpMessageType MessageType;
    };

    std::unordered_map<uint32_t, Frpg2ReliableUdpMessageType> OutstandingResponses;

    uint32_t SentMessageCounter = 0;

    uint32_t LastSentMessageIndex = 0;

    Game* GameInterface;

    static inline size_t DumpMessageIndex = 0;

};