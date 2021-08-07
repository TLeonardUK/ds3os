// Dark Souls 3 - Open Server

#include "Core/Crypto/RSACipher.h"

#include "Core/Utils/Logging.h"

#include <openssl/err.h>

RSACipher::RSACipher(RSAKeyPair* InKey, RSAPaddingMode InPaddingMode)
	: Key(InKey)
	, PaddingMode(InPaddingMode)
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

	int EncryptedLength = RSA_private_encrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	if (!EncryptedLength)
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

	int DecryptedLength = RSA_private_decrypt((int)Input.size(), Input.data(), Output.data(), RsaInstance, OpenSSLPaddingMode);
	if (!DecryptedLength)
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
