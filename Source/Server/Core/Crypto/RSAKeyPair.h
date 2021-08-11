/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "Core/Utils/Endian.h"

#include <filesystem>

#include <openssl/rsa.h>
#include <openssl/pem.h>

// Represents a public/private keypair.

enum class RSAPaddingMode
{
    PKS1_OAEP,
    X931
};

class RSAKeyPair
{
public:

    ~RSAKeyPair();

    bool Generate();
    bool Load(std::filesystem::path& PrivatePath);
    bool Save(std::filesystem::path& PrivatePath, std::filesystem::path& PublicPath);

    //bool Encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output, RSAPaddingMode PaddingMode);
    //bool Decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& Output, RSAPaddingMode PaddingMode);

    std::string GetPrivateString();
    std::string GetPublicString();

    RSA* GetRSA();

private:

    void Cleanup();

    BIGNUM* BigNum = nullptr;
    RSA* RsaInstance = nullptr;

};
