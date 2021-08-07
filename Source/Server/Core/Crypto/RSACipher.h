// Dark Souls 3 - Open Server

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
