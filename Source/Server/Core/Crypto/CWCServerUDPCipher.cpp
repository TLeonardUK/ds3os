/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Crypto/CWCServerUDPCipher.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/Endian.h"
#include "Core/Utils/Strings.h"

// Basically the same as CWCCipher except for different header verification.

CWCServerUDPCipher::CWCServerUDPCipher(const std::vector<uint8_t>& InKey, uint64_t InAuthToken)
    : Key(InKey)
    , AuthToken(InAuthToken)
{
    cwc_init_and_key(InKey.data(), (unsigned long)InKey.size(), &CwcContext);

    // Auth token bytes to encode in the header are the reversed auth token.
    uint8_t* InAuthTokenBytes = reinterpret_cast<uint8_t*>(&InAuthToken);
    AuthTokenHeaderBytes.assign(InAuthTokenBytes, InAuthTokenBytes + 8);
}

bool CWCServerUDPCipher::Encrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> IV(11, 0);
    std::vector<uint8_t> Tag(16, 0);
    std::vector<uint8_t> Payload = Input;

    FillRandomBytes(IV);

    std::vector<uint8_t> Header;
    Header.resize(11);
    memcpy(Header.data(), IV.data(), 11);

    if (cwc_encrypt_message(IV.data(), 11, Header.data(), (unsigned long)Header.size(), (unsigned char*)Payload.data(), (unsigned long)Payload.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    Output.resize(Payload.size() + 11 + 16);

    memcpy(Output.data(), IV.data(), 11);
    memcpy(Output.data() + 11, Tag.data(), 16);
    memcpy(Output.data() + 11 + 16, Payload.data(), Payload.size());

    //Log("EncryptServer: PayloadSize=%i IV=%s Tag=%s Header=%s", Payload.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str());

    return true;
}

bool CWCServerUDPCipher::Decrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> IV(11);
    std::vector<uint8_t> Tag(16);

    // Actually enough data for any data?
    if (Input.size() < 11 + 16 + 1)
    {
        return false;
    }

    Output.resize(Input.size() - 11 - 16);

    memcpy(IV.data(), Input.data(), 11);
    memcpy(Tag.data(), Input.data() + 11, 16);
    memcpy(Output.data(), Input.data() + 11 + 16, Output.size());

    std::vector<uint8_t> Header;
    Header.resize(11);
    memcpy(Header.data(), IV.data(), 11);

    //Log("DecryptServer: PayloadSize=%i IV=%s Tag=%s Header=%s", Output.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str());

    if (cwc_decrypt_message(IV.data(), 11, Header.data(), (unsigned long)Header.size(), (unsigned char*)Output.data(), (unsigned long)Output.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    return true;
}
