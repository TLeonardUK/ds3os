/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Crypto/CWCClientUDPCipher.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/Endian.h"
#include "Core/Utils/Strings.h"

#include <cstring>

// Basically the same as CWCCipher except we include a packet-type and auth token.

CWCClientUDPCipher::CWCClientUDPCipher(const std::vector<uint8_t>& InKey, uint64_t InAuthToken)
    : Key(InKey)
    , AuthToken(InAuthToken)
{
    cwc_init_and_key(InKey.data(), (unsigned long)InKey.size(), &CwcContext);

    // Auth token bytes to encode in the header are the reversed auth token.
    uint8_t* InAuthTokenBytes = reinterpret_cast<uint8_t*>(&InAuthToken);
    AuthTokenHeaderBytes.assign(InAuthTokenBytes, InAuthTokenBytes + 8);
}

bool CWCClientUDPCipher::Encrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> AuthToken = AuthTokenHeaderBytes;
    std::vector<uint8_t> IV(11, 0);
    std::vector<uint8_t> Tag(16, 0);
    std::vector<uint8_t> Payload = Input;
    std::vector<uint8_t> PacketType = { (uint8_t)PacketsHaveConnectionPrefix };

    FillRandomBytes(IV);

    // TODO: I have the distinct feeling this is different when replying as the packet type
    //       doesn't get sent when going server->client ...
    std::vector<uint8_t> Header;
    Header.resize(20);
    memcpy(Header.data(), IV.data(), 11);
    memcpy(Header.data() + 11, AuthToken.data(), 8);
    memcpy(Header.data() + 19, PacketType.data(), 1);

    if (cwc_encrypt_message(IV.data(), 11, Header.data(), (unsigned long)Header.size(), (unsigned char*)Payload.data(), (unsigned long)Payload.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    Output.resize(Payload.size() + 8 + 11 + 16 + 1);

    memcpy(Output.data(), AuthToken.data(), 8);
    memcpy(Output.data() + 8, IV.data(), 11);
    memcpy(Output.data() + 8 + 11, Tag.data(), 16);
    memcpy(Output.data() + 8 + 11 + 16, PacketType.data(), 1);
    memcpy(Output.data() + 8 + 11 + 16 + 1, Payload.data(), Payload.size());

    //Log("EncryptClient: PayloadSize=%i IV=%s Tag=%s Header=%s PacketType=%i", Payload.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str(), PacketType[0]);

    return true;
}

bool CWCClientUDPCipher::Decrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> AuthToken(8);
    std::vector<uint8_t> IV(11);
    std::vector<uint8_t> Tag(16);
    std::vector<uint8_t> PacketType(1);

    // Actually enough data for any data?
    if (Input.size() < 8 + 11 + 16 + 1 + 1)
    {
        return false;
    }

    Output.resize(Input.size() - 8 - 11 - 16 - 1);

    memcpy(AuthToken.data(), Input.data(), 8);
    memcpy(IV.data(), Input.data() + 8, 11);
    memcpy(Tag.data(), Input.data() + 8 + 11, 16);
    memcpy(PacketType.data(), Input.data() + 8 + 11 + 16, 1);
    memcpy(Output.data(), Input.data() + 8 + 11 + 16 + 1, Output.size());

    std::vector<uint8_t> Header;
    Header.resize(20);
    memcpy(Header.data(), IV.data(), 11);
    memcpy(Header.data() + 11, AuthToken.data(), 8);
    memcpy(Header.data() + 19, PacketType.data(), 1);

    //Log("DecryptClient: PayloadSize=%i IV=%s Tag=%s Header=%s PacketType=%i", Output.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str(), PacketType[0]);

    if (cwc_decrypt_message(IV.data(), 11, Header.data(), (unsigned long)Header.size(), (unsigned char*)Output.data(), (unsigned long)Output.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    return true;
}
