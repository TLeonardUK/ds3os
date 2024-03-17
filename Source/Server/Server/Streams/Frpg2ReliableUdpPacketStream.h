/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Server/Streams/Frpg2UdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpPacket.h"

#include <unordered_set>

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
    Frpg2ReliableUdpPacketStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken, bool AsClient = false);

    // Returns true if send was successful, if false is returned the send queue
    // is likely saturated or the packet is invalid.
    virtual bool Send(const Frpg2ReliableUdpPacket& Packet);

    // Notifies us that a packet has been handled and if a reply has been sent or not. This
    // allows us to know if we can now send an ACK for it or not. This is janky and only required
    // because of the stupid difference between ACK and DAT_ACK.
    void HandledPacket(uint32_t AckSequence);

    // Returns true if a packet was recieved and stores packet in OutputPacket.
    virtual bool Recieve(Frpg2ReliableUdpPacket* Packet);

    // Overridden so we can do package retransmission/general management.
    virtual bool Pump() override;

    // Gets the current connection state of this message stream.
    Frpg2ReliableUdpStreamState GetState() { return State; }

    // Attempts to do the initial connection establishing handshake.
    void Connect(const std::string& ClientSteamId);

    // Attempts to do a graceful disconnect so the remote end doesn't send us messages in future.
    void Disconnect();

    // Diassembles a messages into a human-readable string.
    std::string Disassemble(const Frpg2ReliableUdpPacket& Packet);

    // Sends a heartbeat to the remote server.
    void Heartbeat();

protected:

    bool DecodeReliablePacket(const Frpg2UdpPacket& Packet, Frpg2ReliableUdpPacket& Message);
    bool EncodeReliablePacket(const Frpg2ReliableUdpPacket& Message, Frpg2UdpPacket& Packet);

    void HandleIncoming();
    void ConsumeIncomingPackets();
    void HandleIncomingPacket(const Frpg2ReliableUdpPacket& Packet);
    void ProcessPacket(const Frpg2ReliableUdpPacket& Packet);

    void HandleOutgoing();

    void Handle_SYN(const Frpg2ReliableUdpPacket& Packet);
    void Handle_SYN_ACK(const Frpg2ReliableUdpPacket& Packet);
    void Handle_DAT(const Frpg2ReliableUdpPacket& Packet);
    void Handle_HBT(const Frpg2ReliableUdpPacket& Packet);
    void Handle_FIN(const Frpg2ReliableUdpPacket& Packet);
    void Handle_FIN_ACK(const Frpg2ReliableUdpPacket& Packet);
    void Handle_RST(const Frpg2ReliableUdpPacket& Packet);
    void Handle_DAT_ACK(const Frpg2ReliableUdpPacket& Packet);
    void Handle_ACK(const Frpg2ReliableUdpPacket& Packet);
    void Handle_RACK(const Frpg2ReliableUdpPacket& Packet);

    bool SendRaw(const Frpg2ReliableUdpPacket& Packet);

    void Send_SYN();
    void Send_SYN_ACK(uint32_t RemoteIndex);
    void Send_ACK(uint32_t RemoteIndex);
    void Send_DAT_ACK(uint32_t LocalIndex, uint32_t RemoteIndex);
    void Send_FIN_ACK(uint32_t RemoteIndex);
    void Send_FIN();
    void Send_HBT();

    int GetPacketIndexByLocalSequence(const std::vector<Frpg2ReliableUdpPacket>& Queue, uint32_t SequenceIndex);

    bool IsOpcodeSequenced(Frpg2ReliableUdpOpCode Opcode);

    void EmitDebugInfo(bool Incoming, const Frpg2ReliableUdpPacket& Packet);

    virtual void Reset();

    uint32_t GetNextRemoteSequenceIndex() { return (RemoteSequenceIndex + 1) % MAX_ACK_VALUE; }

protected:

    Frpg2ReliableUdpStreamState State = Frpg2ReliableUdpStreamState::Listening;

    double LastPacketRecievedTime = 0.0;
    double LastAckSendTime = 0.0;

    double LastHeartbeatTime = 0.0f;

    std::string SteamId = "";

    // Ack sequences that we have sent replies with DAT_ACK, used to determine
    // what we need to send back in HandledPacket();
    std::unordered_set<uint32_t> DatAckResponses;

    // DAT packets that we except to reply to with a DAT_ACK.
    std::unordered_set<uint32_t> ExpectedDatAckResponses;

    // TODO: Need to handle these rolling over. They have 12 bits so 
    //       it shouldn't happen for a while, but still needs fixing.

    uint32_t SequenceIndex = 0;
    uint32_t SequenceIndexAcked = 0;

    uint32_t RemoteSequenceIndex = 0;
    uint32_t RemoteSequenceIndexAcked = 0;

    bool IsRetransmitting = false;
    uint32_t RetransmittingIndex = 0;
    double RetransmissionTimer = 0.0;
    uint32_t RetransmitAttempts = 0;
    Frpg2ReliableUdpPacket RetransmitPacket;

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

    double ResendSynTimer = 0.0;

    // We stop sending packets and queue them up until we start recieving acks.
    const int MAX_PACKETS_IN_FLIGHT = 32;

    // We reeeeeeeaaaallly want this to be exponential backoff, but this works for now.
    const float RETRANSMIT_INTERVAL = 1.0f;             // 1000 ms

    const float RETRANSMIT_CYCLE_INTERVAL = 0.2f;       // 200 ms

#ifdef _DEBUG
    // Makes debugging easier.
    const uint32_t RETRANSMIT_MAX_ATTEMPTS = std::numeric_limits<uint32_t>::max();        
#else
    const uint32_t RETRANSMIT_MAX_ATTEMPTS = 160;       // 160 * 0.2 = Will give up after trying to retransmiting for 30 seconds 
#endif

    const float RESEND_SYN_INTERVAL = 0.5f;

    const double MIN_TIME_BETWEEN_RESEND_ACK = 0.15;

    // How many seconds to wait for a graceful disconnection.
    const double CONNECTION_CLOSE_TIMEOUT = 5.0;

    // How many values ACK increases before it rolls over.
    const uint32_t MAX_ACK_VALUE = 4096;

    // Top quater ack range, used to handle overflows.
    const uint32_t MAX_ACK_VALUE_TOP_QUART = (4096 / 4) * 3;

    // Bottom quater ack range, used to handle overflows.
    const uint32_t MAX_ACK_VALUE_BOTTOM_QUART = (4096 / 4) * 1;

    double CloseTimer = 0.0f;

    uint32_t LastPacketLocalAck = 0;
    uint32_t LastPacketRemoteAck = 0;

};
