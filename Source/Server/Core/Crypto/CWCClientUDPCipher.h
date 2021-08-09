// Dark Souls 3 - Open Server

#pragma once

#include "Core/Crypto/Cipher.h"

#include "cwc.h"

#include <vector>

class CWCClientUDPCipher
    : public Cipher
{
public:

    CWCClientUDPCipher(const std::vector<uint8_t>& key, uint64_t AuthToken);

    bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;
    bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output) override;

private:
    std::vector<uint8_t> Key;

    cwc_ctx CwcContext;
    
    uint64_t AuthToken;
    std::vector<uint8_t> AuthTokenHeaderBytes;

};
