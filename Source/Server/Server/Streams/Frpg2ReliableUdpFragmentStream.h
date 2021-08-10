// Dark Souls 3 - Open Server

#pragma once

#include "Server/Streams/Frpg2ReliableUdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpFragment.h"

class RSAKeyPair;
class Cipher;

class Frpg2ReliableUdpFragmentStream
    : public Frpg2ReliableUdpPacketStream
{
public:
    Frpg2ReliableUdpFragmentStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // Returns true if send was successful, if false is returned the send queue
    // is likely saturated or the packet is invalid.
    virtual bool Send(const Frpg2ReliableUdpFragment& Fragment);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpFragment* Fragment);

    // Overridden so we can do package retransmission/general management.
    virtual bool Pump() override;

protected:

    virtual bool RecieveInternal(Frpg2ReliableUdpFragment* Fragment);

    bool DecodeFragment(const Frpg2ReliableUdpPacket& Packet, Frpg2ReliableUdpFragment& Fragment);
    bool EncodeFragment(const Frpg2ReliableUdpFragment& Fragment, Frpg2ReliableUdpPacket& Packet);

    virtual void Reset() override;

private:

    std::vector<Frpg2ReliableUdpFragment> Fragments;
    uint32_t RecievedFragmentLength = 0;

    std::vector<Frpg2ReliableUdpFragment> RecieveQueue;
    
    uint32_t SentFragmentCounter = 0;

    // Includes header + compressed payload
    const int MAX_FRAGMENT_LENGTH = 1024;

    const int MIN_SIZE_FOR_COMPRESSION = 256;

};