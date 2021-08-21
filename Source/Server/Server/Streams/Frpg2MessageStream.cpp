/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Server/Streams/Frpg2MessageStream.h"
#include "Server/Streams/Frpg2Message.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Crypto/RSACipher.h"

Frpg2MessageStream::Frpg2MessageStream(std::shared_ptr<NetConnection> Connection, RSAKeyPair* InEncryptionKey)
    : Frpg2PacketStream(Connection)
    , EncryptionKey(InEncryptionKey)
{
    EncryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::X931);
    DecryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::PKS1_OAEP);
}

void Frpg2MessageStream::SetCipher(std::shared_ptr<Cipher> Encryption, std::shared_ptr<Cipher> Decryption)
{
    EncryptionCipher = Encryption;
    DecryptionCipher = Decryption;
}

bool Frpg2MessageStream::Send(const Frpg2Message& Message, uint32_t ResponseToRequestIndex)
{
    // Fill in the header of the message.
    Frpg2Message SendMessage = Message;
    SendMessage.Header.request_index = ResponseToRequestIndex;
    //SendMessage.Header.unknown_1 = ???; // TODO: We need to figure out how to fill this in.

    std::vector<uint8_t> DecryptedBuffer = Message.Payload;
    if (EncryptionCipher)
    {
        if (!EncryptionCipher->Encrypt(DecryptedBuffer, SendMessage.Payload))
        {
            Warning("[%s] Failed to encrypt message payload.", Connection->GetName().c_str());
            return false;
        }
    }

    Frpg2Packet Packet;
    if (!MessageToPacket(SendMessage, Packet))
    {
        Warning("[%s] Failed to convert message to packet payload.", Connection->GetName().c_str());
        return false;
    }

    if (!Frpg2PacketStream::Send(Packet))
    {
        Warning("[%s] Failed to send.", Connection->GetName().c_str());
        return false;
    }

    return true;
}

bool Frpg2MessageStream::Send(google::protobuf::MessageLite* Message, uint32_t ResponseToRequestIndex)
{
    Frpg2Message ResponseMessage;
    ResponseMessage.Payload.resize(Message->ByteSize());

    if (!Message->SerializeToArray(ResponseMessage.Payload.data(), (int)ResponseMessage.Payload.size()))
    {
        Warning("[%s] Failed to serialize protobuf payload.", Connection->GetName().c_str());
        return false;
    }

    if (!Send(ResponseMessage, ResponseToRequestIndex))
    {
        return true;
    }

    return true;
}

bool Frpg2MessageStream::Recieve(Frpg2Message* Message)
{
    Frpg2Packet Packet;
    if (!Frpg2PacketStream::Recieve(&Packet))
    {
        return false;
    }

    if (!PacketToMessage(Packet, *Message))
    {
        Warning("[%s] Failed to convert packet payload to message.", Connection->GetName().c_str());
        return false;
    }

    std::vector<uint8_t> EncryptedBuffer = Message->Payload;
    if (DecryptionCipher)
    {
        if (!DecryptionCipher->Decrypt(EncryptedBuffer, Message->Payload))
        {
            Warning("[%s] Failed to decrypt message payload.", Connection->GetName().c_str());
            return false;
        }
    }

    return true;
}

bool Frpg2MessageStream::PacketToMessage(const Frpg2Packet& Packet, Frpg2Message& Message)
{
    if (Packet.Payload.size() < sizeof(Frpg2MessageHeader))
    {
        Warning("[%s] Packet payload is less than the minimum size of a message, failed to deserialize.", Connection->GetName().c_str());
        return false;
    }

    memcpy(&Message.Header, Packet.Payload.data(), sizeof(Frpg2MessageHeader));
    Message.Header.SwapEndian();

    // This seems the correct way to determine if we a a response, its what darksouls3 
    // seems to do in the dissassembly.
    int PayloadOffset = sizeof(Frpg2MessageHeader);
    if (Message.Header.unknown_1 == 0)
    {
        PayloadOffset += sizeof(Frpg2MessageResponseHeader);

        if (Packet.Payload.size() < sizeof(Frpg2MessageHeader) + sizeof(Frpg2MessageResponseHeader))
        {
            Warning("[%s] Packet payload is less than the minimum size of a message, failed to deserialize.", Connection->GetName().c_str());
            return false;
        }

        memcpy(&Message.ResponseHeader, Packet.Payload.data() + sizeof(Frpg2MessageHeader), sizeof(Frpg2MessageResponseHeader));
        Message.ResponseHeader.SwapEndian();
    }

    size_t PayloadLength = Packet.Payload.size() - PayloadOffset;
    Message.Payload.resize(PayloadLength);
    memcpy(Message.Payload.data(), Packet.Payload.data() + PayloadOffset, PayloadLength);

    return true;
}

bool Frpg2MessageStream::MessageToPacket(const Frpg2Message& Message, Frpg2Packet& Packet)
{
    Frpg2Message ByteSwappedMessage = Message;
    ByteSwappedMessage.Header.SwapEndian();
    ByteSwappedMessage.ResponseHeader.SwapEndian();

    size_t NewSize = sizeof(Frpg2MessageHeader) + Message.Payload.size();
    if (ByteSwappedMessage.Header.unknown_1 == 0)
    {
        NewSize += sizeof(Frpg2MessageResponseHeader);
    }

    Packet.Payload.resize(NewSize);

    memcpy(Packet.Payload.data(), &ByteSwappedMessage.Header, sizeof(Frpg2MessageHeader));

    int PayloadOffset = sizeof(Frpg2MessageHeader);
    if (ByteSwappedMessage.Header.unknown_1 == 0)
    {
        PayloadOffset += sizeof(Frpg2MessageResponseHeader);
        memcpy(Packet.Payload.data() + sizeof(Frpg2MessageHeader), &ByteSwappedMessage.ResponseHeader, sizeof(Frpg2MessageResponseHeader));
    }

    memcpy(Packet.Payload.data() + PayloadOffset, Message.Payload.data(), Message.Payload.size());

    return true;
}
