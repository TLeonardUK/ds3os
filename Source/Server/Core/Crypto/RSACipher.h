/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Crypto/Cipher.h"
#include "Core/Crypto/RSAKeyPair.h"

#include <vector>

class RSACipher
    : public Cipher
{
public:

    RSACipher(RSAKeyPair* Key, RSAPaddingMode PaddingMode);

    bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;
    bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;

private:
    RSAKeyPair* Key;
    RSAPaddingMode PaddingMode;

};
