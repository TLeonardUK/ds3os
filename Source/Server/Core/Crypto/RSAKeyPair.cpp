// Dark Souls 3 - Open Server

#include "Core/Crypto/RSAKeyPair.h"

#include "Core/Utils/Logging.h"

#include <openssl/err.h>

RSAKeyPair::~RSAKeyPair()
{
	Cleanup();
}

void RSAKeyPair::Cleanup()
{
	if (BigNum)
	{
		BN_free(BigNum);
		BigNum = nullptr;
	}
	if (RsaInstance)
	{
		RSA_free(RsaInstance);
		RsaInstance = nullptr;
	}
}

bool RSAKeyPair::Generate()
{
	if (RsaInstance != nullptr)
	{
		Cleanup();
	}

	BigNum = BN_new();
	RsaInstance = RSA_new();

	// Create bignum that our primes will be stored in.
	int Result = BN_set_word(BigNum, RSA_F4);
	if (Result != 1) 
	{
		return false;
	}

	// Generate primes for key. 
	Result = RSA_generate_key_ex(RsaInstance, 2048, BigNum, NULL);
	if (Result != 1) 
	{
		return false;
	}

    return true;
}

bool RSAKeyPair::Load(std::filesystem::path& PrivatePath)
{
	if (RsaInstance != nullptr)
	{
		Cleanup();
	}

	// Read private key.
	BIO* BioPrivate = BIO_new_file(PrivatePath.string().c_str(), "r+");
	RsaInstance = PEM_read_bio_RSAPrivateKey(BioPrivate, &RsaInstance, nullptr, nullptr);
	BIO_free_all(BioPrivate);

	if (RsaInstance == nullptr)
	{
		return false;
	}

    return true;
}

bool RSAKeyPair::Save(std::filesystem::path& PrivatePath, std::filesystem::path& PublicPath)
{
	// Save public key.
	BIO* BioPublic = BIO_new_file(PublicPath.string().c_str(), "w+");
	int Result = PEM_write_bio_RSAPublicKey(BioPublic, RsaInstance);
	BIO_free_all(BioPublic);

	if (Result != 1)
	{
		return false;
	}

	// Save private key.
	BIO* BioPrivate = BIO_new_file(PrivatePath.string().c_str(), "w+");
	Result = PEM_write_bio_RSAPrivateKey(BioPrivate, RsaInstance, nullptr, nullptr, 0, nullptr, nullptr);
	BIO_free_all(BioPublic);

	if (Result != 1)
	{
		return false;
	}

	return true;
}

std::string RSAKeyPair::GetPrivateString()
{
	BIO* BioPrivate = BIO_new(BIO_s_mem());
	int Result = PEM_write_bio_RSAPrivateKey(BioPrivate, RsaInstance, nullptr, nullptr, 0, nullptr, nullptr);

	if (Result != 1)
	{
		BIO_free_all(BioPrivate);
		return "";
	}
	
	std::vector<char> Buffer;
	Buffer.resize(4096);

	int BytesRead = BIO_read(BioPrivate, Buffer.data(), (int)Buffer.size());
	if (BytesRead <= 0)
	{
		BIO_free_all(BioPrivate);
		return "";
	}

	Buffer[BytesRead] = '\0';

	BIO_free_all(BioPrivate);
	return (std::string)Buffer.data();
}

std::string RSAKeyPair::GetPublicString()
{
	BIO* BioPublic = BIO_new(BIO_s_mem());
	int Result = PEM_write_bio_RSAPublicKey(BioPublic, RsaInstance);

	if (Result != 1)
	{
		BIO_free_all(BioPublic);
		return "";
	}

	std::vector<char> Buffer;
	Buffer.resize(4096);

	int BytesRead = BIO_read(BioPublic, Buffer.data(), (int)Buffer.size());
	if (BytesRead <= 0)
	{
		BIO_free_all(BioPublic);
		return "";
	}

	Buffer[BytesRead] = '\0';

	BIO_free_all(BioPublic);
	return (std::string)Buffer.data();
}

RSA* RSAKeyPair::GetRSA()
{
	return RsaInstance;
}