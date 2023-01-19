/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Shared/Core/Crypto/RSACipher.h"

#include "Shared/Core/Utils/Logging.h"

#include <openssl/err.h>

RSACipher::RSACipher(RSAKeyPair* InKey, RSAPaddingMode InPaddingMode, bool InUsePublicKeyToEncrypt)
	: Key(InKey)
	, PaddingMode(InPaddingMode)
	, UsePublicKeyToEncrypt(InUsePublicKeyToEncrypt)
{
}

bool RSACipher::Encrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
	RSA* RsaInstance = Key->GetRSA();

	Output.resize(RSA_size(RsaInstance));

	int OpenSSLPaddingMode = RSA_PKCS1_OAEP_PADDING;
	switch (PaddingMode)
	{
	case RSAPaddingMode::PKS1_OAEP:
		OpenSSLPaddingMode = RSA_PKCS1_OAEP_PADDING;
		break;
	case RSAPaddingMode::X931:
		OpenSSLPaddingMode = RSA_X931_PADDING;
		break;
	}

	int EncryptedLength = 0;
	if (UsePublicKeyToEncrypt)
	{
		EncryptedLength = RSA_public_encrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	}
	else
	{
		EncryptedLength = RSA_private_encrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	}

	if (EncryptedLength < 0)
	{
		std::vector<char> buffer;
		buffer.resize(1024);

		ERR_load_crypto_strings();
		ERR_error_string_n(ERR_get_error(), buffer.data(), buffer.size());

		Error("Failed to encrypt RSA message with error: [%lu](%s)", EncryptedLength, buffer.data());

		return false;
	}

	Output.resize(EncryptedLength);

	return true;
}

bool RSACipher::Decrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
	RSA* RsaInstance = Key->GetRSA();

	Output.resize(RSA_size(RsaInstance));

	int OpenSSLPaddingMode = RSA_PKCS1_OAEP_PADDING;
	switch (PaddingMode)
	{
	case RSAPaddingMode::PKS1_OAEP:
		OpenSSLPaddingMode = RSA_PKCS1_OAEP_PADDING;
		break;
	case RSAPaddingMode::X931:
		OpenSSLPaddingMode = RSA_X931_PADDING;
		break;
	}

	int DecryptedLength = 0;
	if (UsePublicKeyToEncrypt)
	{
		DecryptedLength = RSA_public_decrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	}
	else
	{
		DecryptedLength = RSA_private_decrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	}
	if (DecryptedLength < 0)
	{
		std::vector<char> buffer;
		buffer.resize(1024);

		ERR_load_crypto_strings();
		ERR_error_string_n(ERR_get_error(), buffer.data(), buffer.size());

		Error("Failed to decrypt RSA message with error: [%lu](%s)", DecryptedLength, buffer.data());

		return false;
	}

	Output.resize(DecryptedLength);

	return true;
}
