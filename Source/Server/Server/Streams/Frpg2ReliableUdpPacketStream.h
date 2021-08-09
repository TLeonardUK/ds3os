// Dark Souls 3 - Open Server

#pragma once

#include "Server/Streams/Frpg2UdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpPacket.h"

struct Frpg2ReliableUdpPacket;
class RSAKeyPair;
class Cipher;

// This packet stream handles the core reliable udp packet 
// transmission. Higher level functionality like packet fragmentation,
// compression, etc is all handled at the higher level 
// Frpg2ReliableUdpMessageStream.

class Frpg2ReliableUdpPacketStream
    : public Frpg2UdpPacketStream
{
public:
    Frpg2ReliableUdpPacketStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken);

    // Returns true if send was successful, if false is returned the send queue
    // is likely saturated or the packet is invalid.
    virtual bool Send(const Frpg2ReliableUdpPacket& Packet);

    // Sends a packet replying to a previous packet.
    // For each Recieve either SendReply or SendAck should be called for it.
    virtual bool SendReply(const Frpg2ReliableUdpPacket& Response, const Frpg2ReliableUdpPacket& ReplyingTo);

    // Sends a packet acknowledging reciept of a packet.
    // For each Recieve either SendReply or SendAck should be called for it.
    virtual bool SendAck(const Frpg2ReliableUdpPacket& ReplyingTo);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpPacket* Packet);

    // Overridden so we can do package retransmission/general management.
    virtual bool Pump() override;

protected:

    bool DecodeReliablePacket(const Frpg2UdpPacket& Packet, Frpg2ReliableUdpPacket& Message);
    bool EncodeReliablePacket(const Frpg2ReliableUdpPacket& Message, Frpg2UdpPacket& Packet);

    void HandleIncoming();
    void HandleIncomingPacket(const Frpg2ReliableUdpPacket& Packet);
    void ProcessPacket(const Frpg2ReliableUdpPacket& Packet);

    void HandleOutgoing();

    void Handle_SYN(const Frpg2ReliableUdpPacket& Packet);
    void Handle_DAT(const Frpg2ReliableUdpPacket& Packet);
    void Handle_HBT(const Frpg2ReliableUdpPacket& Packet);
    void Handle_FIN(const Frpg2ReliableUdpPacket& Packet);
    void Handle_RST(const Frpg2ReliableUdpPacket& Packet);
    void Handle_DAT_ACK(const Frpg2ReliableUdpPacket& Packet);
    void Handle_ACK(const Frpg2ReliableUdpPacket& Packet);

    bool SendRaw(const Frpg2ReliableUdpPacket& Packet);

    void Send_SYN_ACK(uint32_t RemoteIndex);
    void Send_ACK(uint32_t RemoteIndex);
    void Send_DAT_ACK(const Frpg2ReliableUdpPacket& Response, uint32_t RemoteIndex);
    void Send_HBT();

    int GetPacketIndexByLocalSequence(const std::vector<Frpg2ReliableUdpPacket>& Queue, uint32_t SequenceIndex);
    void InsertPacketByLocalSequence(std::vector<Frpg2ReliableUdpPacket>& Queue, const Frpg2ReliableUdpPacket& Packet, uint32_t SequenceIndex);

    bool IsOpcodeSequenced(Frpg2ReliableUdpOpCode Opcode);

    void Reset();

protected:

    Frpg2ReliableUdpStreamState State = Frpg2ReliableUdpStreamState::Listening;

    double LastPacketRecievedTime = 0.0;
    double LastAckSendTime = 0.0;

    // TODO: Need to handle these rolling over. They have 12 bits so 
    //       it shouldn't happen for a while, but still needs fixing.

    uint32_t SequenceIndex = 1;
    uint32_t SequenceIndexAcked = 0;

    uint32_t RemoteSequenceIndex = 0;
    uint32_t RemoteSequenceIndexAcked = 0;

    bool IsRetransmitting = false;
    uint32_t RetransmittingIndex = 0;

    // TODO: All these should be shared pointers or something, we do way
    //       too much data shuffling with raw packets.

    // Packets that have been recieved and are awaiting processing. They will
    // stay in this queue until they are the next in the remote sequence index.
    std::vector<Frpg2ReliableUdpPacket> PendingRecieveQueue;

    // Ordered packets read for whoever calls Recieve() to handle.
    std::vector<Frpg2ReliableUdpPacket> RecieveQueue;

    // Packets that are queued to send, will be sent when transmission is permitted.
    std::vector<Frpg2ReliableUdpPacket> SendQueue;

    // Queue of packets that have been send but not acknowledged yet, held on to 
    // until they have been acked.
    std::vector<Frpg2ReliableUdpPacket> RetransmitBuffer; // Use priority queue.

    // We stop sending packets and queue them up until we start recieving acks.
    const int MAX_PACKETS_IN_FLIGHT = 10;

    // We reeeeeeeaaaallly want this to be exponential backoff, but this works for now.
    const float RETRANSMIT_INTERVAL = 0.5;

    const double MIN_TIME_BETWEEN_RESEND_ACK = 0.1;

};