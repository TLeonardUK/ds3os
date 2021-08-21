/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2ReliableUdpPacketStream.h"
#include "Server/Streams/Frpg2ReliableUdpPacket.h"
#include "Server/Streams/Frpg2UdpPacket.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/File.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Crypto/RSACipher.h"

Frpg2ReliableUdpPacketStream::Frpg2ReliableUdpPacketStream(std::shared_ptr<NetConnection> Connection, const std::vector<uint8_t>& CwcKey, uint64_t AuthToken)
    : Frpg2UdpPacketStream(Connection, CwcKey, AuthToken)
{
}

void Frpg2ReliableUdpPacketStream::Disconnect()
{
    if (State == Frpg2ReliableUdpStreamState::Established)
    {
        Send_FIN();
    }
}

bool Frpg2ReliableUdpPacketStream::Send(const Frpg2ReliableUdpPacket& Input)
{
    // Swallow any packets being sent while we are closing.
    if (State == Frpg2ReliableUdpStreamState::Closing)
    {
        return true;
    }

    if (IsOpcodeSequenced(Input.Header.opcode) || Input.Header.opcode == Frpg2ReliableUdpOpCode::Unset)
    {
        Frpg2ReliableUdpPacket SentPacket = Input;
        SentPacket.SendTime = GetSeconds();

        // Opcode note set, we fill in the opcode and ack counters then
        // otherwise we assume the sender has dealt with it.
        if (SentPacket.Header.opcode == Frpg2ReliableUdpOpCode::Unset)
        {
            uint32_t Local, Remote;
            SentPacket.Header.GetAckCounters(Local, Remote);

            SentPacket.Header.SetAckCounters(SequenceIndex, Remote);

            if (Remote > 0)
            {
                SentPacket.Header.opcode = Frpg2ReliableUdpOpCode::DAT_ACK;
                DatAckResponses.insert(Remote);

                RemoteSequenceIndexAcked = std::max(RemoteSequenceIndex, Remote);
            }
            else
            {
                SentPacket.Header.opcode = Frpg2ReliableUdpOpCode::DAT;
            }
        }

        SequenceIndex++;
        
        SendQueue.push_back(SentPacket);
    }
    else
    {
        SendRaw(Input);
    }

    return true;
}

bool Frpg2ReliableUdpPacketStream::Recieve(Frpg2ReliableUdpPacket* Output)
{
    if (RecieveQueue.size() > 0)
    {
        *Output = RecieveQueue[0];
        RecieveQueue.erase(RecieveQueue.begin());

        return true;
    }

    return false;
}

bool Frpg2ReliableUdpPacketStream::DecodeReliablePacket(const Frpg2UdpPacket& Input, Frpg2ReliableUdpPacket& Output)
{
    if (Input.Payload.size() < sizeof(Frpg2ReliableUdpPacketHeader))
    {
        Warning("[%s] Packet payload is less than the minimum size of a message, failed to deserialize.", Connection->GetName().c_str());
        InErrorState = true;
        return false;
    }

    int HeaderOffset = 0;
    int PayloadOffset = sizeof(Frpg2ReliableUdpPacketHeader);
    int PayloadSize = (int)Input.Payload.size() - sizeof(Frpg2ReliableUdpPacketHeader);
    if (Input.Payload[0] != 0xF5 || Input.Payload[1] != 0x02)
    {
        // TODO: This is a hack to find the correct packet header offset, this only occurs for the very first syn packet so we
        //       can hard code this. We should do this a better way though.
        HeaderOffset = (int)Input.Payload.size() - sizeof(Frpg2ReliableUdpPacketHeader) - sizeof(Frpg2ReliableUdpPacketOpCodePayload_SYN);
        PayloadOffset = 0;

        Ensure(Input.Payload[HeaderOffset] == 0xF5 && Input.Payload[HeaderOffset + 1] == 0x02);
    }

    Output.Payload.resize(PayloadSize);

    memcpy(&Output.Header, Input.Payload.data() + HeaderOffset, sizeof(Frpg2ReliableUdpPacketHeader));
    memcpy(Output.Payload.data(), Input.Payload.data() + PayloadOffset, PayloadSize);

    //Output.Header.SwapEndian();

    return true;
}

bool Frpg2ReliableUdpPacketStream::EncodeReliablePacket(const Frpg2ReliableUdpPacket& Input, Frpg2UdpPacket& Output)
{
    Frpg2ReliableUdpPacket ByteSwappedMessage = Input;
    //ByteSwappedMessage.Header.SwapEndian();

    Output.Payload.resize(sizeof(Frpg2ReliableUdpPacketHeader) + Input.Payload.size());

    memcpy(Output.Payload.data(), &ByteSwappedMessage.Header, sizeof(Frpg2ReliableUdpPacketHeader));
    memcpy(Output.Payload.data() + sizeof(Frpg2ReliableUdpPacketHeader), Input.Payload.data(), Input.Payload.size());

    return true;
}

void Frpg2ReliableUdpPacketStream::HandleIncoming()
{
    // Accept any packets currently being recieved.
    while (true)
    {
        Frpg2UdpPacket Packet;
        if (!Frpg2UdpPacketStream::Recieve(&Packet))
        {
            break;
        }

        Frpg2ReliableUdpPacket ReliablePacket;
        if (!DecodeReliablePacket(Packet, ReliablePacket))
        {
            Warning("[%s] Failed to convert packet payload to message.", Connection->GetName().c_str());
            InErrorState = true;
            continue;
        }

        HandleIncomingPacket(ReliablePacket);
    }

    // Process as many packets as we can off the pending queue.
    while (PendingRecieveQueue.size() > 0)
    {
        Frpg2ReliableUdpPacket& Next = PendingRecieveQueue[0];
        
        uint32_t Local, Remote;
        Next.Header.GetAckCounters(Local, Remote);

        if (Local == RemoteSequenceIndex + 1)
        {
            //Log("[%s] Processing next packet in sequence %i.", Connection->GetName().c_str(), RemoteSequenceIndex + 1);

            ProcessPacket(Next);
      
            PendingRecieveQueue.erase(PendingRecieveQueue.begin());
            RemoteSequenceIndex++;
        }
        else
        {
            break;
        }
    }
}

int Frpg2ReliableUdpPacketStream::GetPacketIndexByLocalSequence(const std::vector<Frpg2ReliableUdpPacket>& Queue, uint32_t SequenceIndex)
{
    for (size_t i = 0; i < Queue.size(); i++)
    {
        uint32_t Local, Remote;
        Queue[i].Header.GetAckCounters(Local, Remote);

        if (Local == SequenceIndex)
        {
            return (int)i;
        }
    }
    return -1;
}

void Frpg2ReliableUdpPacketStream::InsertPacketByLocalSequence(std::vector<Frpg2ReliableUdpPacket>& Queue, const Frpg2ReliableUdpPacket& Packet, uint32_t SequenceIndex)
{
    for (size_t i = 0; i < Queue.size(); i++)
    {
        uint32_t Local, Remote;
        Queue[i].Header.GetAckCounters(Local, Remote);

        if (Local > SequenceIndex)
        {
            Queue.insert(Queue.begin() + i, Packet);
            return;
        }
    }

    Queue.push_back(Packet);
}

bool Frpg2ReliableUdpPacketStream::IsOpcodeSequenced(Frpg2ReliableUdpOpCode Opcode)
{
    // Determines if an opcode causes incrementing of the sequence value and 
    // needs to be queue and sent via the normal retransmission channel. 
    // Otherwise it can be sent raw at any time and the sequence does not matter.

    return Opcode == Frpg2ReliableUdpOpCode::DAT ||
           Opcode == Frpg2ReliableUdpOpCode::DAT_ACK ||
           Opcode == Frpg2ReliableUdpOpCode::SYN_ACK ||
           Opcode == Frpg2ReliableUdpOpCode::FIN_ACK;
}

void Frpg2ReliableUdpPacketStream::HandleIncomingPacket(const Frpg2ReliableUdpPacket& Packet)
{
    LastPacketRecievedTime = GetSeconds();

    uint32_t LocalAck, RemoteAck;
    Packet.Header.GetAckCounters(LocalAck, RemoteAck);

//    Log("[%s] Recieved Packet: LocalAck=%i RemoteAck=%i", Connection->GetName().c_str(), LocalAck, RemoteAck);
    //EmitDebugInfo(true, Packet);

    // Check sequence index to prune duplicate / out of order for relevant packets.
    if (IsOpcodeSequenced(Packet.Header.opcode))
    {
        if (State != Frpg2ReliableUdpStreamState::Established)
        {
            // TODO: Handle situation where the handshake is completed but we recieve a following
            // packet out of order.
            Warning("[%s] Recieved sequenced packets before connection is established, this is not allowed. ", Connection->GetName().c_str());
            InErrorState = true;
            return;
        }

        bool IsDuplicate = false;

        if (LocalAck <= RemoteSequenceIndex)
        {
            Warning("[%s] Ignoring incoming packet, prior to ack head (incoming=%i head=%i), likely duplicate. ", Connection->GetName().c_str(), LocalAck, RemoteSequenceIndex);
            IsDuplicate = true;
        }

        if (GetPacketIndexByLocalSequence(PendingRecieveQueue, LocalAck) >= 0)
        {
            Warning("[%s] Ignoring incoming packet, duplicate that we already have. ", Connection->GetName().c_str());
            IsDuplicate = true;
        }

        if (IsDuplicate && (GetSeconds() - LastAckSendTime) > MIN_TIME_BETWEEN_RESEND_ACK)
        {   
            // Send an ACK, its possible that the remote is retransmitting packets as
            // a previously sent ACK has dropped.
            Log("Sending ack as not sent in a while.");

            Send_ACK(RemoteSequenceIndexAcked);

            return;
        }
        else if (!IsDuplicate)
        {
            InsertPacketByLocalSequence(PendingRecieveQueue, Packet, LocalAck);
        }
    }
    else
    {
        ProcessPacket(Packet);
    }
}

void Frpg2ReliableUdpPacketStream::ProcessPacket(const Frpg2ReliableUdpPacket & Packet)
{
    uint32_t LocalAck, RemoteAck;
    Packet.Header.GetAckCounters(LocalAck, RemoteAck);

    //Log("[%s] Processing Packet: LocalAck=%i RemoteAck=%i", Connection->GetName().c_str(), LocalAck, RemoteAck);

    switch (Packet.Header.opcode)
    {
    case Frpg2ReliableUdpOpCode::SYN:
        {
            Handle_SYN(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::DAT:
        {
            Handle_DAT(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::HBT:
        {
            Handle_HBT(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::FIN:
        {
            Handle_FIN(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::RST:
        {
            Handle_RST(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::ACK:
        {
            Handle_ACK(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::DAT_ACK:
        {
            Handle_DAT_ACK(Packet);
            break;
        }
    case Frpg2ReliableUdpOpCode::FIN_ACK:
    case Frpg2ReliableUdpOpCode::SYN_ACK:
        {
            Error("[%s] Recieved opcode that is normally only used for outgoing connections, which we don't currently support.", Connection->GetName().c_str());
            Ensure(false);
            break;
        }
    default:
        {
            Error("[%s] Recieved unknown reliable udp opcode 0x%2x.", Connection->GetName().c_str(), Packet.Header.opcode);
            Ensure(false);
            break;
        }
    }
}

void Frpg2ReliableUdpPacketStream::Handle_SYN(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved SYN, establishing handshake.", Connection->GetName().c_str());

    State = Frpg2ReliableUdpStreamState::SynRecieved;

    uint32_t InLocalAck, InRemoteAck;
    Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

    // Send a SYN_ACK in response.
    Send_SYN_ACK(InLocalAck);

    // And send our ACK message as well (this seems redundent, but its what happens in ds3).
    Send_ACK(InLocalAck);
}

void Frpg2ReliableUdpPacketStream::Handle_HBT(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved HBT.", Connection->GetName().c_str());

    uint32_t InLocalAck, InRemoteAck;
    Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

    SequenceIndexAcked = std::max(SequenceIndexAcked, InRemoteAck);

    Send_HBT();
}

void Frpg2ReliableUdpPacketStream::Handle_FIN(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved FIN.", Connection->GetName().c_str());

    uint32_t InLocalAck, InRemoteAck;
    Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

    // TODO: We probably need to send a FIN_ACK here as well.
    Send_FIN_ACK(InLocalAck);

    State = Frpg2ReliableUdpStreamState::Closing;
}

void Frpg2ReliableUdpPacketStream::Handle_RST(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved RST.", Connection->GetName().c_str());

    State = Frpg2ReliableUdpStreamState::Listening;
    Reset();
}

void Frpg2ReliableUdpPacketStream::Handle_ACK(const Frpg2ReliableUdpPacket& Packet)
{
    if (State == Frpg2ReliableUdpStreamState::SynRecieved)
    {
        //Log("[%s] Recieved initial ACK, handshake finished connection established.", Connection->GetName().c_str());
        State = Frpg2ReliableUdpStreamState::Established;
    }
    else
    {
        //Log("[%s] Recieved ACK.", Connection->GetName().c_str());
    }

    uint32_t InLocalAck, InRemoteAck;
    Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

    SequenceIndexAcked = std::max(SequenceIndexAcked, InRemoteAck);
}

void Frpg2ReliableUdpPacketStream::Handle_DAT(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved DAT.", Connection->GetName().c_str());

    RecieveQueue.push_back(Packet);
}

void Frpg2ReliableUdpPacketStream::Handle_DAT_ACK(const Frpg2ReliableUdpPacket& Packet)
{
    //Log("[%s] Recieved DAT_ACK.", Connection->GetName().c_str());

    uint32_t InLocalAck, InRemoteAck;
    Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

    SequenceIndexAcked = std::max(SequenceIndexAcked, InRemoteAck);
    
    // TODO: From our perspective do we actually need to do anything special with DAT_ACK? 
    //       Filter up that a packet is a reply to another or something? Most of our traffic
    //       goes through the message stream anyway which already handles this.

    RecieveQueue.push_back(Packet);
}

void Frpg2ReliableUdpPacketStream::Send_SYN_ACK(uint32_t RemoteIndex)
{
    Frpg2ReliableUdpPacket SynAckResponse;
    SynAckResponse.Header.SetAckCounters(SequenceIndex, RemoteIndex);
    SynAckResponse.Header.opcode = Frpg2ReliableUdpOpCode::SYN_ACK;

    // TODO: Figure out these values, they seem to always be the same, but we 
    // should figure out what they are regardless.
    Frpg2ReliableUdpPacketOpCodePayload_SYN SynPayload;
    SynPayload.unknown[0] = 0x12;
    SynPayload.unknown[1] = 0x10;
    SynPayload.unknown[2] = 0x20;
    SynPayload.unknown[3] = 0x20;
    SynPayload.unknown[4] = 0x00;
    SynPayload.unknown[5] = 0x01;
    SynPayload.unknown[6] = 0x00;
    SynPayload.unknown[7] = 0x00;

    SynAckResponse.Payload.resize(sizeof(SynPayload));
    memcpy(SynAckResponse.Payload.data(), &SynPayload, sizeof(SynPayload));

    Send(SynAckResponse);

    RemoteSequenceIndex = RemoteIndex;
}

void Frpg2ReliableUdpPacketStream::Send_ACK(uint32_t RemoteIndex)
{
    Frpg2ReliableUdpPacket AckResponse;
    AckResponse.Header.SetAckCounters(0, RemoteIndex);
    AckResponse.Header.opcode = Frpg2ReliableUdpOpCode::ACK;

    Send(AckResponse);

    RemoteSequenceIndexAcked = std::max(RemoteSequenceIndex, RemoteIndex);
    LastAckSendTime = GetSeconds();
}

void Frpg2ReliableUdpPacketStream::Send_FIN_ACK(uint32_t RemoteIndex)
{
    Frpg2ReliableUdpPacket AckResponse;
    AckResponse.Header.SetAckCounters(SequenceIndex, RemoteIndex);
    AckResponse.Header.opcode = Frpg2ReliableUdpOpCode::FIN_ACK;

    Send(AckResponse);
}

void Frpg2ReliableUdpPacketStream::Send_FIN()
{
    Frpg2ReliableUdpPacket AckResponse;
    AckResponse.Header.SetAckCounters(SequenceIndex, 0);
    AckResponse.Header.opcode = Frpg2ReliableUdpOpCode::FIN;

    Send(AckResponse);

    State = Frpg2ReliableUdpStreamState::Closing;
    CloseTimer = GetSeconds();
}

void Frpg2ReliableUdpPacketStream::Send_HBT()
{
    Frpg2ReliableUdpPacket HbtResponse;
    HbtResponse.Header.SetAckCounters(0, RemoteSequenceIndexAcked);
    HbtResponse.Header.opcode = Frpg2ReliableUdpOpCode::HBT;

    Send(HbtResponse);
}

bool Frpg2ReliableUdpPacketStream::SendRaw(const Frpg2ReliableUdpPacket& Input)
{
    uint32_t LocalAck, RemoteAck;
    Input.Header.GetAckCounters(LocalAck, RemoteAck);

    //Log("[%s] Sent Packet: LocalAck=%i RemoteAck=%i", Connection->GetName().c_str(), LocalAck, RemoteAck);
    //EmitDebugInfo(false, Input);

    Frpg2UdpPacket Packet;
    if (!EncodeReliablePacket(Input, Packet))
    {
        Warning("[%s] Failed to convert message to packet payload.", Connection->GetName().c_str());
        InErrorState = true;
        return false;
    }

    if (!Frpg2UdpPacketStream::Send(Packet))
    {
        Warning("[%s] Failed to send.", Connection->GetName().c_str());
        InErrorState = true;
        return false;
    }

    return true;
}

void Frpg2ReliableUdpPacketStream::Reset()
{
    SequenceIndex = 1;
    SequenceIndexAcked = 0;
    RemoteSequenceIndex = 0;
    RemoteSequenceIndexAcked = 0;

    PendingRecieveQueue.clear();
    RecieveQueue.clear();    
    SendQueue.clear();
    RetransmitBuffer.clear();
}

void Frpg2ReliableUdpPacketStream::HandleOutgoing()
{
    // Trim off any retransmit packets that are not long relevant.
    for (auto iter = RetransmitBuffer.begin(); iter != RetransmitBuffer.end(); /* empty */)
    {
        Frpg2ReliableUdpPacket& Packet = *iter;

        uint32_t InLocalAck, InRemoteAck;
        Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

        if (InLocalAck <= SequenceIndexAcked)
        {
            //Log("[%s] Removing packet for retransmit buffer as its been acknowledged.", Connection->GetName().c_str());

            iter = RetransmitBuffer.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    // If we have not had ack of packets in the retransmit queue for long enough, retransmit 
    // the first one and hope it gets acked soon.
    if (!IsRetransmitting)
    {
        double CurrentTime = GetSeconds();
        for (auto iter = RetransmitBuffer.begin(); iter != RetransmitBuffer.end(); iter++)
        {
            Frpg2ReliableUdpPacket& Packet = *iter;

            uint32_t InLocalAck, InRemoteAck;
            Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

            double ElapsedTime = (CurrentTime - Packet.SendTime);
            if (ElapsedTime > RETRANSMIT_INTERVAL)
            {
                Log("[%s] Starting retransmit as we have unacknowledged packets.", Connection->GetName().c_str());

                SendRaw(Packet);

                IsRetransmitting = true;
                RetransmittingIndex = InLocalAck;
            }
        } 
    }
    else if (SequenceIndexAcked >= RetransmittingIndex)
    {
        Log("[%s] Recovered from retransmit.", Connection->GetName().c_str());

        IsRetransmitting = false;
    }

    // Do not send any packets if we have a lot of packets waiting for ack.
    while (!IsRetransmitting && SendQueue.size() > 0 && RetransmitBuffer.size() < MAX_PACKETS_IN_FLIGHT)
    {
        Frpg2ReliableUdpPacket Packet = SendQueue[0];
        SendQueue.erase(SendQueue.begin());
        RetransmitBuffer.push_back(Packet);

        uint32_t InLocalAck, InRemoteAck;
        Packet.Header.GetAckCounters(InLocalAck, InRemoteAck);

        //Log("[%s] Sending sequenced packet, %i.", Connection->GetName().c_str(), InLocalAck);

        SendRaw(Packet);
    }
}

bool Frpg2ReliableUdpPacketStream::Pump()
{
    // Mark as connection closed after we have sent everything in the queue.
    if (State == Frpg2ReliableUdpStreamState::Closing && SendQueue.size() == 0)
    {
        Log("[%s] Connection closed.", Connection->GetName().c_str());
        State = Frpg2ReliableUdpStreamState::Closed;
    }

    // If connection is now closed, just drop all the packets.
    if (State == Frpg2ReliableUdpStreamState::Closed)
    {
        Reset();
        return true;
    }

    if (Frpg2UdpPacketStream::Pump())
    {
        return true;
    }

    // If closing and its taken too long then don't bother trying to gracefully disconnect.
    if (CloseTimer > 0.0f && State == Frpg2ReliableUdpStreamState::Closing)
    {        
        if (double Elapsed = GetSeconds() - CloseTimer; Elapsed > CONNECTION_CLOSE_TIMEOUT)
        {
            Log("[%s] Connection closing took to long, assuming connection terminated.", Connection->GetName().c_str());
            State = Frpg2ReliableUdpStreamState::Closed;        
            return true;
        }
    }

    HandleIncoming();
    HandleOutgoing();

    // Acknowledge whatever the latest thing recieved was.
    /*if (RemoteSequenceIndex != RemoteSequenceIndexAcked)
    {
        Send_ACK(RemoteSequenceIndex);
    }*/

    return false;
}

void Frpg2ReliableUdpPacketStream::EmitDebugInfo(bool Incoming, const Frpg2ReliableUdpPacket& Packet)
{
    uint32_t Local, Remote;
    Packet.Header.GetAckCounters(Local, Remote);
    Log("%s %-9s %-6i %-6i", Incoming ? "<<" : ">>", ToString(Packet.Header.opcode).c_str(), Local, Remote);
}

void Frpg2ReliableUdpPacketStream::HandledPacket(uint32_t AckSequence)
{
    if (auto iter = DatAckResponses.find(AckSequence); iter != DatAckResponses.end())
    {
        DatAckResponses.erase(iter);
        return;
    }

    Send_ACK(AckSequence);
}