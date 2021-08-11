/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <vector>

class Cipher
{
public:

    virtual ~Cipher() { }

    virtual bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) = 0;
    virtual bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) = 0;

};
