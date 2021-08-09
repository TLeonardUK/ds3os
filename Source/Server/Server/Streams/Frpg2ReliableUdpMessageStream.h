// Dark Souls 3 - Open Server

#pragma once

#include "Server/Streams/Frpg2ReliableUdpPacketStream.h"

#include "Protobuf/Frpg2RequestMessage.pb.h"

struct Frpg2ReliableUdpMessage;
class RSAKeyPair;
class Cipher;

class Frpg2ReliableUdpMessageStream
    : public Frpg2ReliableUdpPacketStream
{
public:
    Frpg2ReliableUdpMessageStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // Returns true if send was successful, if false is returned the send queue
    // is likely saturated or the packet is invalid.
    virtual bool Send(const Frpg2ReliableUdpMessage& Message);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpMessage* Message);

    // Overridden so we can do package retransmission/general management.
    virtual bool Pump() override;

protected:

    virtual bool RecieveInternal(Frpg2ReliableUdpMessage* Message);

    bool DecodeMessage(const Frpg2ReliableUdpPacket& Packet, Frpg2ReliableUdpMessage& Message);
    bool EncodeMessage(const Frpg2ReliableUdpMessage& Message, Frpg2ReliableUdpPacket& Packet);

    virtual void Reset() override;

private:

    std::vector<Frpg2ReliableUdpMessage> Fragments;
    uint32_t RecievedFragmentLength = 0;

    std::vector<Frpg2ReliableUdpMessage> RecieveQueue;
    
    // Includes header + compressed payload
    const int MAX_FRAGMENT_MESSAGE_LENGTH = 1024;

};