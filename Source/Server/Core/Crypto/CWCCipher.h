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

#include "cwc.h"

#include <vector>

class CWCCipher
    : public Cipher
{
public:

    CWCCipher(const std::vector<uint8_t>& key);

    bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;
    bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;

private:
    std::vector<uint8_t> Key;

    cwc_ctx CwcContext;

};
