/*
 * NRSSRSanitizer.h
 * Copyright (C) 2022 William Tremblay
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <inttypes.h>

#ifdef _WIN32
#include <corecrt_wstring.h>
#else
#include <wchar.h>
#include <byteswap.h>
#endif

#include "Server.DarkSouls3/Protobuf/DS3_Protobufs.h"

 // Static class that holds methods intended to validate the structure
 // of the size-delimited entry lists and serialized NRSessionSearchResult 
 // data for security purposes, namely patching out-of-bounds read crashes 
 // and a reliable remote code execution exploit (CVE-2022-24126).
 // 
 // This specifically relates to the game's V1.15 version. It will most 
 // likely have to be reworked if a new client version comes out, but this 
 // should not be necessary as the only reason for a client update would be 
 // patching this bug.
class DS3_NRSSRSanitizer
{
public:

	DS3_NRSSRSanitizer() = delete;

	// The game checks if these two match and rejects the data if they dont, 
	// so we can use them to detect NRSSR data.
	inline static const uint32_t SIGNATURE = 0x5652584E;
	inline static const uint16_t VERSION_NUMBER = 0x8405;

	// The size of the session data information in bytes, stored in big-endian in the packet.
	// This is always 8 on PC and holds the ID of the Steam lobby the client should connect to.
	inline static const uint16_t SESSION_DATA_SIZE = 8;

	// The size of the host online id field in NRSSR data. On PC this is 8 (size of a CSteamID).
	inline static const size_t HOST_ONLINE_ID_SIZE = 8;

	// Game stack buffer sizes for property and name strings (in number of uint16_t (note:not wchar_t!!! wchar_t size is compiler dependent))
	inline static const size_t MAX_PROP_WSTR_SIZE = 1024;
	inline static const size_t MAX_NAME_WSTR_SIZE = 256;

	// Possible outcomes of size-delimited entry list validation
	enum class ValidationResult
	{
		// The entry list data is valid.
		Valid,
		// One of the entry size fields is invalid.
		EntryList_SizeMismatch,
		// The signature or version of NRSSR data does not match expected values.
		NRSSR_SignatureOrVersion_Mismatch,
		// There is not enough data left in the buffer to read NRSSR property metadata.
		NRSSR_PropertyMetadata_InsufficientData,
		// The provided NRSSR property type is invalid.
		NRSSR_PropertyMetadata_InvalidType,
		// There is not enough data left in the buffer to read a 4-byte NRSSR property.
		NRSSR_Property4Byte_InsufficientData,
		// There is not enough data left in the buffer to read an 8-byte NRSRR property.
		NRSSR_Property8Byte_InsufficientData,
		// A string NRSSR property is either longer than the client's buffer or is not null-terminated.
		NRSSR_PropertyString_Overflow,
		// The NRSSR host name string is either longer than the client's buffer or is not null-terminated.
		NRSSR_NameString_Overflow,
		// The amount of data left in the NRSSR struct after the name string is abnormal.
		NRSSR_RemainingDataSize_Mismatch,
		// NRSSR session size field does not match with the expected value.
		NRSSR_SessionSize_Abnormal,
        // Push message being validated is unknown.
        NRSSR_PushMessage_UnknownType,
        // Unable to parse push messages.
        NRSSR_PushMessage_ParseFailure,
	};

	// Verify if length-delimited entry list data used to store session join information is valid.
	// Will also validate any NRSSR data stored in the entries.
	static ValidationResult ValidateEntryList(const uint8_t* EntryList, size_t Size)
	{
		// Go over the entries in the entry list and make sure any NRSSR entry is valid
		size_t i = 0;
		for (size_t EntrySize = 0; i + 8 <= Size; i += EntrySize + 8)
		{
			EntrySize = *reinterpret_cast<const uint32_t*>(EntryList + i + 4);
			if (i + 8 + EntrySize > Size) return ValidationResult::EntryList_SizeMismatch;

			const uint8_t* EntryData = EntryList + i + 8;
			// Use the NRSSR signature and version to detect NRSSR data inside the entry list.
			// This may result in valid packets getting flagged, but from my testing it has never happened.
			if (CheckNRSSRSignatureAndVersion(EntryData, EntrySize))
			{
				ValidationResult Result = ValidateNRSSRData(EntryData, EntrySize);
				if (Result != ValidationResult::Valid) return Result;
			}
		}
		// Make sure there is no data left is the buffer after the final entry, i.e. every byte has been
		// accounted for
		return (i == Size) ? ValidationResult::Valid : ValidationResult::EntryList_SizeMismatch;
	}

	static ValidationResult ValidateEntryList(const char* EntryList, size_t Size)
	{
		return ValidateEntryList(reinterpret_cast<const uint8_t*>(EntryList), Size);
	}

	// Verify if some data has correct NRSessionSearchResult signature and version number.
	static bool CheckNRSSRSignatureAndVersion(const uint8_t* Data, size_t Size)
	{
		if (Size < 6) return false;
		uint32_t Signature = *reinterpret_cast<const uint32_t*>(Data + 0);
		uint16_t Version = *reinterpret_cast<const uint16_t*>(Data + 4);
		return Signature == SIGNATURE && Version == VERSION_NUMBER;
	}

	// Secure functions do not exist on anything else but msvc. The normal functions
	// have slightly different behaviour. So just adding this quickly to match behaviour
	// between platforms.
	static size_t internal_wcsnlen_s(const uint16_t* str, size_t strsz)
	{
		if (str == nullptr)
		{
			return 0;
		}

		for (size_t i = 0; i < strsz; i++)
		{
			if (str[i] == '\0')
			{
				return i;
			}
		}

		return strsz;
	}

	// Verify if serialized NRSessionSearchResult data is valid. 
	static ValidationResult ValidateNRSSRData(const uint8_t* NRSSRData, size_t Size)
	{
		// Check if signature and version number matches
		if (!CheckNRSSRSignatureAndVersion(NRSSRData, Size))
		{
			return ValidationResult::NRSSR_SignatureOrVersion_Mismatch;
		}

		// Make sure that we have enough data to read the property count field 
		if (Size < 7)
		{
			return ValidationResult::NRSSR_PropertyMetadata_InsufficientData;
		}
		uint8_t PropertyCount = NRSSRData[6];

		// Parse the property list and verify each entry has a valid type and length
		int Position = 7, StrLength, NumWideCharLeft, InternalStrLength;
		for (int i = 0; i < PropertyCount; i++)
		{
			// We don't care about the ID or unknown value, just check sizes
			if (Size - Position < 6)
			{
				return ValidationResult::NRSSR_PropertyMetadata_InsufficientData;
			}
			uint8_t Type = NRSSRData[Position + 4];
			Position += 6;

			switch (Type)
			{
			case 1: // Case 1 : 4 byte field
				if (Size - Position < 4)
				{
					return ValidationResult::NRSSR_Property4Byte_InsufficientData;
				}
				Position += 4;
				break;
			case 2:
			case 3: // Cases 2-3 : 8 byte field (perhaps signed/unsigned?)
				if (Size - Position < 8)
				{
					return ValidationResult::NRSSR_Property8Byte_InsufficientData;
				}
				Position += 8;
				break;
			case 4: // Case 4 : Null-terminated wide string field
				NumWideCharLeft = (Size - Position) / 2;
				StrLength = internal_wcsnlen_s(reinterpret_cast<const uint16_t*>(NRSSRData + Position), NumWideCharLeft);
				if (StrLength >= NumWideCharLeft || StrLength >= MAX_PROP_WSTR_SIZE)
				{
					return ValidationResult::NRSSR_PropertyString_Overflow;
				}
				Position += 2 * (StrLength + 1);
				break;
			default:
				return ValidationResult::NRSSR_PropertyMetadata_InvalidType;
			}
		}

		// Check if the host name is null terminated and has valid length
		NumWideCharLeft = (Size - Position) / 2;
		StrLength = internal_wcsnlen_s(reinterpret_cast<const uint16_t*>(NRSSRData + Position), NumWideCharLeft);
		if (StrLength >= NumWideCharLeft || StrLength >= MAX_NAME_WSTR_SIZE)
		{
			return ValidationResult::NRSSR_NameString_Overflow;
		}
		Position += 2 * (StrLength + 1);

		// Check host online id and session data size		
		if (Size - Position != sizeof(SESSION_DATA_SIZE) + HOST_ONLINE_ID_SIZE + SESSION_DATA_SIZE)
		{
			return ValidationResult::NRSSR_RemainingDataSize_Mismatch;
		}

		// Read the big-endian sessin data size field and make sure it matches the normal value for the game
		uint16_t SessionDataSize = (uint16_t)NRSSRData[Position + HOST_ONLINE_ID_SIZE] << 8 | (uint16_t)NRSSRData[Position + HOST_ONLINE_ID_SIZE + 1];
		if (SessionDataSize == SESSION_DATA_SIZE)
		{
			return ValidationResult::Valid;
		}
		else
		{
			return ValidationResult::NRSSR_SessionSize_Abnormal;
		}
	}

    static ValidationResult ValidatePushMessages(const std::string& Data)
    {
        DS3_Frpg2RequestMessage::PushRequestHeader Header;
        if (!Header.ParseFromString(Data))
        {
            return ValidationResult::NRSSR_PushMessage_UnknownType;
        }

        switch (Header.push_message_id())
        {
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestAllowBreakInTarget:
            {
                DS3_Frpg2RequestMessage::PushRequestAllowBreakInTarget Msg;
                if (!Msg.ParseFromString(Data))
                {
                    return ValidationResult::NRSSR_PushMessage_ParseFailure;
                }

                auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Msg.player_struct().data(), Msg.player_struct().size());
                if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
                {
                    return ValidationResult;
                }

                break;
            }
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestVisit:
            {
                DS3_Frpg2RequestMessage::PushRequestVisit Msg;
                if (!Msg.ParseFromString(Data))
                {
                    return ValidationResult::NRSSR_PushMessage_ParseFailure;
                }

                auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Msg.data().data(), Msg.data().size());
                if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
                {
                    return ValidationResult;
                }

                break;
            }
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestAcceptQuickMatch:
            {
                DS3_Frpg2RequestMessage::AcceptQuickMatchMessage Msg;
                if (!Msg.ParseFromString(Data))
                {
                    return ValidationResult::NRSSR_PushMessage_ParseFailure;
                }

                auto ValidationResult = DS3_NRSSRSanitizer::ValidateEntryList(Msg.metadata().data(), Msg.metadata().size());
                if (ValidationResult != DS3_NRSSRSanitizer::ValidationResult::Valid)
                {
                    return ValidationResult;
                }

                break;
            }
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRemoveSign:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestSummonSign:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRejectSign:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestJoinQuickMatch:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRejectQuickMatch:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PlayerInfoUploadConfigPushMessage:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestEvaluateBloodMessage:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestBreakInTarget:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRejectBreakInTarget:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRejectVisit:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestRemoveVisitor:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_PushRequestNotifyRingBell:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_RegulationFileUpdatePushMessage:
        case DS3_Frpg2RequestMessage::PushMessageId::PushID_ManagementTextMessage:
            {   
                // These messages don't look to have anything that needs validating in them.
                break;
            }
        default:
            {
                return ValidationResult::NRSSR_PushMessage_UnknownType;
            }
        }        

        return ValidationResult::Valid;
    }
};