// Dark Souls 3 - Open Server

#include "Core/Crypto/CWCUDPCipher.h"

#include "Core/Utils/Logging.h"
#include "Core/Utils/Random.h"
#include "Core/Utils/Endian.h"
#include "Core/Utils/Strings.h"

CWCUDPCipher::CWCUDPCipher(const std::vector<uint8_t>& InKey, uint64_t InAuthToken)
    : Key(InKey)
    , AuthToken(InAuthToken)
{
    cwc_init_and_key(InKey.data(), InKey.size(), &CwcContext);

    // Auth token bytes to encode in the header are the reversed auth token.
    uint8_t* InAuthTokenBytes = reinterpret_cast<uint8_t*>(&InAuthToken);
    AuthTokenHeaderBytes.assign(InAuthTokenBytes, InAuthTokenBytes + 8);
}

bool CWCUDPCipher::Encrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> IV(11, 0);
    std::vector<uint8_t> Tag(16, 0);
    std::vector<uint8_t> Payload = Input;
    std::vector<uint8_t> PacketType = { 0 };

    FillRandomBytes(IV);

    // TODO: I have the distinct feeling this is different when replying as the packet type
    //       doesn't get sent when going server->client ...
    std::vector<uint8_t> Header;
    Header.resize(20);
    memcpy(Header.data(), IV.data(), 11);
    memcpy(Header.data() + 11, AuthTokenHeaderBytes.data(), 8);
    memcpy(Header.data() + 19, PacketType.data(), 1);

    if (cwc_encrypt_message(IV.data(), 11, Header.data(), Header.size(), (unsigned char*)Payload.data(), Payload.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    Output.resize(Payload.size() + 11 + 16 + 1);

    memcpy(Output.data(), IV.data(), 11);
    memcpy(Output.data() + 11, Tag.data(), 16);
    memcpy(Output.data() + 11 + 16, PacketType.data(), 1);
    memcpy(Output.data() + 11 + 16 + 1, Payload.data(), Payload.size());

    Log("Encrypt: PayloadSize=%i IV=%s Tag=%s Header=%s", Payload.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str());

    return true;
}

bool CWCUDPCipher::Decrypt(const std::vector<uint8_t>& Input, std::vector<uint8_t>& Output)
{
    std::vector<uint8_t> IV(11);
    std::vector<uint8_t> Tag(16);
    std::vector<uint8_t> PacketType(1);

    // Actually enough data for any data?
    if (Input.size() < 11 + 16 + 1 + 1)
    {
        return false;
    }

    Output.resize(Input.size() - 11 - 16 - 1);

    memcpy(IV.data(), Input.data(), 11);
    memcpy(Tag.data(), Input.data() + 11, 16);
    memcpy(PacketType.data(), Input.data() + 11 + 16, 1);
    memcpy(Output.data(), Input.data() + 11 + 16 + 1, Output.size());

    std::vector<uint8_t> Header;
    Header.resize(20);
    memcpy(Header.data(), IV.data(), 11);
    memcpy(Header.data() + 11, AuthTokenHeaderBytes.data(), 8);
    memcpy(Header.data() + 19, PacketType.data(), 1);

    Log("Decrypt: PayloadSize=%i IV=%s Tag=%s Header=%s", Output.size(), BytesToHex(IV).c_str(), BytesToHex(Tag).c_str(), BytesToHex(Header).c_str());

    if (cwc_decrypt_message(IV.data(), 11, Header.data(), Header.size(), (unsigned char*)Output.data(), Output.size(), Tag.data(), 16, &CwcContext) == RETURN_ERROR)
    {
        return false;
    }

    return true;
}
