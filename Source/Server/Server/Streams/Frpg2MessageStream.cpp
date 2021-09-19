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

#include "Config/BuildConfig.h"

#include "Core/Network/NetConnection.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Strings.h"

#include "Core/Crypto/RSAKeyPair.h"
#include "Core/Crypto/RSACipher.h"

Frpg2MessageStream::Frpg2MessageStream(std::shared_ptr<NetConnection> Connection, RSAKeyPair* InEncryptionKey, bool AsClient)
    : Frpg2PacketStream(Connection)
    , EncryptionKey(InEncryptionKey)
{
    if (AsClient)
    {
        EncryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::PKS1_OAEP, true);
        DecryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::X931, true);
    }
    else
    {
        EncryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::X931, false);
        DecryptionCipher = std::make_shared<RSACipher>(EncryptionKey, RSAPaddingMode::PKS1_OAEP, false);
    }
}

void Frpg2MessageStream::SetCipher(std::shared_ptr<Cipher> Encryption, std::shared_ptr<Cipher> Decryption)
{
    EncryptionCipher = Encryption;
    DecryptionCipher = Decryption;
}

bool Frpg2MessageStream::Send(const Frpg2Message& Message, Frpg2MessageType MessageType, uint32_t ResponseToRequestIndex)
{
    Frpg2Packet Packet;

    // Fill in the header of the message.
    Frpg2Message SendMessage = Message;
    SendMessage.Header.msg_index = ResponseToRequestIndex;
    SendMessage.Header.msg_type = MessageType;

    // Disassemble if required.
    if constexpr (BuildConfig::DISASSEMBLE_SENT_MESSAGES)
    {
        Packet.Disassembly = Disassemble(SendMessage);
    }

    std::vector<uint8_t> DecryptedBuffer = Message.Payload;
    if (EncryptionCipher)
    {
        if (!EncryptionCipher->Encrypt(DecryptedBuffer, SendMessage.Payload))
        {
            WarningS(Connection->GetName().c_str(), "Failed to encrypt message payload.");
            return false;
        }
    }

    if (!MessageToPacket(SendMessage, Packet))
    {
        WarningS(Connection->GetName().c_str(), "Failed to convert message to packet payload.");
        return false;
    }

    if (!Frpg2PacketStream::Send(Packet))
    {
        WarningS(Connection->GetName().c_str(), "Failed to send.");
        return false;
    }

    return true;
}

bool Frpg2MessageStream::Send(google::protobuf::MessageLite* Message, Frpg2MessageType MessageType, uint32_t ResponseToRequestIndex)
{
    Frpg2Message ResponseMessage;
    ResponseMessage.Payload.resize(Message->ByteSize());

    if (!Message->SerializeToArray(ResponseMessage.Payload.data(), (int)ResponseMessage.Payload.size()))
    {
        WarningS(Connection->GetName().c_str(), "Failed to serialize protobuf payload.");
        return false;
    }

    if (!Send(ResponseMessage, MessageType, ResponseToRequestIndex))
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
        WarningS(Connection->GetName().c_str(), "Failed to convert packet payload to message.");
        return false;
    }

    std::vector<uint8_t> EncryptedBuffer = Message->Payload;
    if (DecryptionCipher && EncryptedBuffer.size() > 0)
    {
        if (!DecryptionCipher->Decrypt(EncryptedBuffer, Message->Payload))
        {
            WarningS(Connection->GetName().c_str(), "Failed to decrypt message payload.");
            return false;
        }
    }

    // Disassemble if required.
    if constexpr (BuildConfig::DISASSEMBLE_RECIEVED_MESSAGES)
    {
        Message->Disassembly = Packet.Disassembly;
        Message->Disassembly.append(Disassemble(*Message));

        Log("\n<< RECV\n%s", Message->Disassembly.c_str());
    }

    return true;
}

bool Frpg2MessageStream::PacketToMessage(const Frpg2Packet& Packet, Frpg2Message& Message)
{
    if (Packet.Payload.size() < sizeof(Frpg2MessageHeader))
    {
        WarningS(Connection->GetName().c_str(), "Packet payload is less than the minimum size of a message, failed to deserialize.");
        return false;
    }

    memcpy(&Message.Header, Packet.Payload.data(), sizeof(Frpg2MessageHeader));
    Message.Header.SwapEndian();

    // This seems the correct way to determine if we a a response, its what darksouls3 
    // seems to do in the dissassembly.
    int PayloadOffset = sizeof(Frpg2MessageHeader);
    if (Message.Header.msg_type == Frpg2MessageType::Reply)
    {
        PayloadOffset += sizeof(Frpg2MessageResponseHeader);

        if (Packet.Payload.size() < sizeof(Frpg2MessageHeader) + sizeof(Frpg2MessageResponseHeader))
        {
            WarningS(Connection->GetName().c_str(), "Packet payload is less than the minimum size of a message, failed to deserialize.");
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
    if (ByteSwappedMessage.Header.msg_type == Frpg2MessageType::Reply)
    {
        NewSize += sizeof(Frpg2MessageResponseHeader);
    }

    Packet.Payload.resize(NewSize);

    memcpy(Packet.Payload.data(), &ByteSwappedMessage.Header, sizeof(Frpg2MessageHeader));

    int PayloadOffset = sizeof(Frpg2MessageHeader);
    if (ByteSwappedMessage.Header.msg_type == Frpg2MessageType::Reply)
    {
        PayloadOffset += sizeof(Frpg2MessageResponseHeader);
        memcpy(Packet.Payload.data() + sizeof(Frpg2MessageHeader), &ByteSwappedMessage.ResponseHeader, sizeof(Frpg2MessageResponseHeader));
    }

    memcpy(Packet.Payload.data() + PayloadOffset, Message.Payload.data(), Message.Payload.size());

    return true;
}

std::string Frpg2MessageStream::Disassemble(const Frpg2Message& Message)
{
    std::string Result = "";

    Result += "Message:\n";
    Result += StringFormat("\t%-30s = %u\n", "header_size", Message.Header.header_size);
    Result += StringFormat("\t%-30s = %u\n", "msg_type", Message.Header.msg_type);
    Result += StringFormat("\t%-30s = %u\n", "msg_index", Message.Header.msg_index);

    if (Message.Header.msg_type == Frpg2MessageType::Reply)
    {
        Result += StringFormat("\t%-30s = %u\n", "unknown_1", Message.ResponseHeader.unknown_1);
        Result += StringFormat("\t%-30s = %u\n", "unknown_2", Message.ResponseHeader.unknown_2);
        Result += StringFormat("\t%-30s = %u\n", "unknown_3", Message.ResponseHeader.unknown_3);
        Result += StringFormat("\t%-30s = %u\n", "unknown_4", Message.ResponseHeader.unknown_4);
    }

    Result += "Message Payload:\n";
    Result += BytesToString(Message.Payload, "\t");

    return Result;
}