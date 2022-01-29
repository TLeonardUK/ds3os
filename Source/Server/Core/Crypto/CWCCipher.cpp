/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Core/Crypto/CWCCipher.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/Endian.h"
#include "Core/Utils/Strings.h"

#include <cstring>

CWCCipher::CWCCipher(const std::vector<uint8_t>& InKey)
    : Key(InKey)
{
    cwc_init_and_key(InKey.data(), (unsigned long)InKey.size(), &CwcContext);
}

bool CWCCipher::Encrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> IV(11, 0);
    std::vector<uint8_t> Tag(16, 0);
    std::vector<uint8_t> Payload = Input;

    FillRandomBytes(IV);

    if (cwc_encrypt_message(IV.data(), 11, IV.data(), 11, (unsigned char*)Payload.data(), (unsigned long)Payload.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    Output.resize(Payload.size() + 11 + 16);

    memcpy(Output.data(), IV.data(), 11);
    memcpy(Output.data() + 11, Tag.data(), 16);
    memcpy(Output.data() + 11 + 16, Payload.data(), Payload.size());

    return true;
}

bool CWCCipher::Decrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
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

    if (cwc_decrypt_message(IV.data(), 11, IV.data(), 11, (unsigned char*)Output.data(), (unsigned long)Output.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    return true;
}
