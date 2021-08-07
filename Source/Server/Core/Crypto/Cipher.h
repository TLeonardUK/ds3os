// Dark Souls 3 - Open Server

#pragma once

#include <vector>

class Cipher
{
public:

    virtual ~Cipher() { }

    virtual bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) = 0;
    virtual bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) = 0;

};
