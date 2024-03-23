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

 // Static class that holds methods intended to validate the structure
 // of the size-delimited entry lists and serialized NRSessionSearchResult 
 // data for security purposes, namely patching out-of-bounds read crashes 
 // and a reliable remote code execution exploit (CVE-2022-24126).
 // 
 // This specifically relates to the game's V1.15 version. It will most 
 // likely have to be reworked if a new client version comes out, but this 
 // should not be necessary as the only reason for a client update would be 
 // patching this bug.
class DS2_NRSSRSanitizer
{
public:

	DS2_NRSSRSanitizer() = delete;

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
		NRSSR_SessionSize_Abnormal
	};

	// Verify if length-delimited entry list data used to store session join information is valid.
	// Will also validate any NRSSR data stored in the entries.
	static ValidationResult ValidateEntryList(const uint8_t* EntryList, size_t Size)
	{
        // Format is different in DS2. 
        // If we want to enable support for older versions, we should decipher it.
        return ValidationResult::Valid;
	}

	static ValidationResult ValidateEntryList(const char* EntryList, size_t Size)
	{
		return ValidateEntryList(reinterpret_cast<const uint8_t*>(EntryList), Size);
	}

    static ValidationResult ValidatePushMessages(const std::string& Data)
    {
        // Format is different in DS2. 
        // If we want to enable support for older versions, we should decipher it.
        return ValidationResult::Valid;
    }
};