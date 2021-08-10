// Dark Souls 3 - Open Server

#pragma once

#include "Server/Streams/Frpg2ReliableUdpFragmentStream.h"
#include "Server/Streams/Frpg2ReliableUdpMessage.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

#include <unordered_map>

class Cipher;

class Frpg2ReliableUdpMessageStream
    : public Frpg2ReliableUdpFragmentStream
{
public:
    Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // Short hand version of Send for protobufs, takes care of constructing the wrapper message.
    virtual bool Send(google::protobuf::MessageLite* Message, const Frpg2ReliableUdpMessage* ResponseTo = nullptr);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpMessage* Message);

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

};