/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#include "Injector/Hooks/DarkSouls2/DS2_LogProtobufsHook.h"
#include "Injector/Config/BuildConfig.h"
#include "Injector/Injector/Injector.h"
#include "Shared/Core/Utils/Logging.h"
#include "Shared/Core/Utils/Strings.h"
#include "Shared/Core/Utils/Rtti.h"
#include "Shared/Core/Utils/Protobuf.h"
#include "Shared/Core/Utils/File.h"
#include "ThirdParty/detours/src/detours.h"

#include <vector>
#include <iterator>
#include <string>

namespace
{
    DecodedProtobufRegistry s_decoded_registry;
    std::atomic_size_t s_packet_count{0};

    using SerializeWithCachedSizesToArray_p = uint8_t*(*)(void* this_ptr, uint8_t* target);
    SerializeWithCachedSizesToArray_p s_original_SerializeWithCachedSizesToArray;

    uint8_t* SerializeWithCachedSizesToArrayHook(void* this_ptr, uint8_t* target)
    {
        std::string RttiName = GetRttiNameFromObject(this_ptr);
        std::string ClassName = RttiName;
        if (size_t pos = ClassName.find_last_of(':'); pos != std::string::npos)
        {
            ClassName = ClassName.substr(pos + 1);
        }

        // Log that this protobuf was sent.
        Log(">> %s", RttiName.c_str());

        // Pass over to original serialization function to encode.
        uint8_t* end = s_original_SerializeWithCachedSizesToArray(this_ptr, target);

        // Store protobuf on disk if needed.
        if constexpr (BuildConfig::WRITE_OUT_PROTOBUFS)
        {
            std::vector<uint8_t> bytes(target, end);
            WriteBytesToFile(StringFormat("%s/%llu_%s.bin", BuildConfig::TEMP_LOG_FOLDER, s_packet_count.fetch_add(1), ClassName.c_str()), bytes);
        }

        // Store decoded protobuf.
        s_decoded_registry.Decode(ClassName, target, end - target);

        // Store decoded protobuf on disk if needed.
        if constexpr (BuildConfig::WRITE_OUT_DECODED_PROTOBUFS)
        {
            WriteTextToFile(StringFormat("%s/decoded.proto", BuildConfig::TEMP_LOG_FOLDER), s_decoded_registry.ToString());
        }        

        return end;
    }

    using ParseFromArray_p = bool (*)(void* this_ptr, void* data, int size);
    ParseFromArray_p s_original_ParseFromArray;

    bool ParseFromArrayHook(void* this_ptr, void* data, int size)
    {
        std::string RttiName = GetRttiNameFromObject(this_ptr);
        std::string ClassName = RttiName;
        if (size_t pos = ClassName.find_last_of(':'); pos != std::string::npos)
        {
            ClassName = ClassName.substr(pos + 1);
        }

        // Log that this protobuf was recieved.
        Log("<< %s", RttiName.c_str());

        // Store protobuf on disk if needed.
        if constexpr (BuildConfig::WRITE_OUT_PROTOBUFS)
        {
            std::vector<uint8_t> bytes((uint8_t*)data, (uint8_t*)data + size);
            WriteBytesToFile(StringFormat("%s/%llu_%s.bin", BuildConfig::TEMP_LOG_FOLDER, s_packet_count.fetch_add(1), ClassName.c_str()), bytes);
        }

        // Store decoded version of the protobuf.
        s_decoded_registry.Decode(ClassName, (uint8_t*)data, size);

        // Store decoded protobuf on disk if needed.
        if constexpr (BuildConfig::WRITE_OUT_DECODED_PROTOBUFS)
        {
            WriteTextToFile(StringFormat("%s/decoded.proto", BuildConfig::TEMP_LOG_FOLDER), s_decoded_registry.ToString());
        }

        // Pass over the original parsing function.
        bool result = s_original_ParseFromArray(this_ptr, data, size);
        if (!result)
        {
            Error("!! Failed to parse incoming protobuf, check format from ds3os.");
        }

        return result;
    }
};

bool DS2_LogProtobufsHook::Install_SerializeWithCachedSizesToArray(Injector& injector)
{
    // This is the prolog of SerializeWithCachedSizesToArray
    std::vector<intptr_t> matches = injector.SearchAOB({
        0x40, 0x55,
        0x56,
        0x57,
        0x48, 0x81, 0xec, 0xd0, 0x00, 0x00, 0x00,
        0x48, 0xc7, 0x44, 0x24, 0x28, 0xfe, 0xff, 0xff, 0xff,
        0x48, 0x89, 0x9c, 0x24, 0x00, 0x01, 0x00, 0x00,
        {}, {}, {}, {}, {}, {}, {}, // skip abs address.
        0x48, 0x33, 0xc4,
        0x48, 0x89, 0x84, 0x24, 0xc0, 0x00, 0x00, 0x00,
        0x48, 0x8b, 0xf2,
        0x48, 0x8b, 0xd9,
        0x33, 0xff,
        0x89, 0x7c, 0x24, 0x24,
        0x48, 0x8b, 0x01,
        0xff, 0x50, 0x58,
        0x48, 0x63, 0xe8,
        0x41, 0x83, 0xc9, 0xff,
        0x44, 0x8b, 0xc5,
        0x48, 0x8b, 0xd6,
        0x48, 0x8d, 0x4c, 0x24, 0x50
        });

    if (matches.size() == 0)
    {
        Error("Failed to find injection point for logging protobufs.");
        return false;
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    s_original_SerializeWithCachedSizesToArray = reinterpret_cast<SerializeWithCachedSizesToArray_p>(matches[0]);
    DetourAttach(&(PVOID&)s_original_SerializeWithCachedSizesToArray, SerializeWithCachedSizesToArrayHook);

    DetourTransactionCommit();

    return true;
}

bool DS2_LogProtobufsHook::Install_ParseFromArray(Injector& injector)
{
    // This is the prolog of ParseFromArray
    std::vector<intptr_t> matches = injector.SearchAOB({
        0x49, 0x8b, 0xc0,
        0x4c, 0x8b, 0xca,
        0x4c, 0x8b, 0xc1,
        0x48, 0x8b, 0xd0,
        0x49, 0x8b, 0xc9,
        0xe9, 0xac, 0xfe,
        0xff, 0xff,
    });

    if (matches.size() == 0)
    {
        Error("Failed to find injection point for logging protobufs.");
        return false;
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    s_original_ParseFromArray = reinterpret_cast<ParseFromArray_p>(matches[0]);
    DetourAttach(&(PVOID&)s_original_ParseFromArray, ParseFromArrayHook);

    DetourTransactionCommit();

    return true;
}

bool DS2_LogProtobufsHook::Install(Injector& injector)
{
    return Install_SerializeWithCachedSizesToArray(injector) &&
           Install_ParseFromArray(injector);
}

void DS2_LogProtobufsHook::Uninstall()
{

}

const char* DS2_LogProtobufsHook::GetName()
{
    return "DS2 Log Protobufs";
}

