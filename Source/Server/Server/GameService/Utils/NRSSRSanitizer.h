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
#include <corecrt_wstring.h>

// Static class that holds methods intended to validate the structure
// of the size-delimited entry lists and serialized NRSessionSearchResult 
// data for security purposes, namely patching out-of-bounds read crashes 
// and a reliable remote code execution exploit (CVE-2022-24126).
// 
// This specifically relates to the game's V1.15 version. It will most 
// likely have to be reworked if a new client version comes out, but this 
// should not be necessary as the only reason for a client update would be 
// patching this bug.
class NRSSRSanitizer
{
public:

	NRSSRSanitizer() = delete;

	// The game checks if these two match and rejects the data if they dont, 
	// so we can use them to detect NRSSR data.
	inline static const uint32_t SIGNATURE = 0x5652584E;
	inline static const uint16_t VERSION_NUMBER = 0x8405;

	// The size of the session data information in bytes, stored in big-endian in the packet.
	// This is always 8 on PC and holds the ID of the Steam lobby the client should connect to.
	inline static const uint16_t SESSION_DATA_SIZE = 8;

	// The size of the host online id field in NRSSR data. On PC this is 8 (size of a CSteamID).
	inline static const size_t HOST_ONLINE_ID_SIZE = 8;

	// Game stack buffer sizes for property and name strings (in number of wchar_t)
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
	static ValidationResult IsValidEntryList(const uint8_t* entry_list, size_t size)
	{
		// Go over the entries in the entry list and make sure any NRSSR entry is valid
		size_t i = 0;
		for (size_t entry_sz = 0; i + 8 <= size; i += entry_sz + 8)
		{
			entry_sz = *(uint32_t*)(entry_list + i + 4);
			if (i + 8 + entry_sz > size) return ValidationResult::EntryList_SizeMismatch;

			const uint8_t* entry_data = entry_list + i + 8;
			// Use the NRSSR signature and version to detect NRSSR data inside the entry list.
			// This may result in valid packets getting flagged, but from my testing it has never happened.
			if (CheckNRSSRSignatureAndVersion(entry_data, entry_sz))
			{
				ValidationResult res = IsValidNRSSRData(entry_data, entry_sz);
				if (res != ValidationResult::Valid) return res;
			}
		}
		// Make sure there is no data left is the buffer after the final entry, i.e. every byte has been
		// accounted for
		return (i == size) ? ValidationResult::Valid : ValidationResult::EntryList_SizeMismatch;
	}

	static ValidationResult IsValidEntryList(const char* entry_list, size_t size)
	{
		return IsValidEntryList((const uint8_t*)entry_list, size);
	}

	// Verify if some data has correct NRSessionSearchResult signature and version number.
	static bool CheckNRSSRSignatureAndVersion(const uint8_t* data, size_t size)
	{
		if (size < 6) return false;
		uint32_t sig = *(uint32_t*)(data + 0);
		uint16_t ver = *(uint16_t*)(data + 4);
		return sig == SIGNATURE && ver == VERSION_NUMBER;
	}

	// Verify if serialized NRSessionSearchResult data is valid. 
	static ValidationResult IsValidNRSSRData(const uint8_t* nrssr_data, size_t size)
	{
		// Check if signature and version number matches
		if (!CheckNRSSRSignatureAndVersion(nrssr_data, size)) return ValidationResult::NRSSR_SignatureOrVersion_Mismatch;

		// Make sure that we have enough data to read the property count field 
		if (size < 7) return ValidationResult::NRSSR_PropertyMetadata_InsufficientData;;
		uint8_t prop_cnt = *(uint8_t*)(nrssr_data + 6);

		// Parse the property list and verify each entry has a valid type and length
		size_t pos = 7, len;
		for (int i = 0; i < prop_cnt; i++)
		{	// We don't care about the ID or unknown value, just check sizes
			if (size - pos < 6) return ValidationResult::NRSSR_PropertyMetadata_InsufficientData;
			uint8_t type = nrssr_data[pos + 4];
			pos += 6;

			switch (type)
			{
			case 1: // Case 1 : 4 byte field
				if (size - pos < 4) return ValidationResult::NRSSR_Property4Byte_InsufficientData;
				pos += 4;
				break;
			case 2:
			case 3: // Cases 2-3 : 8 byte field (perhaps signed/unsigned?)
				if (size - pos < 8) return ValidationResult::NRSSR_Property8Byte_InsufficientData;
				pos += 8;
				break;
			case 4: // Case 4 : Null-terminated wide string field
				len = wcsnlen_s((wchar_t*)(nrssr_data + pos), size - pos);
				if (len == size - pos || len >= MAX_PROP_WSTR_SIZE) return ValidationResult::NRSSR_PropertyString_Overflow;
				pos += 2 * (len + 1);
				break;
			default:
				return ValidationResult::NRSSR_PropertyMetadata_InvalidType;
			}
		}

		// Check host name
		len = wcsnlen_s((wchar_t*)(nrssr_data + pos), size - pos);
		if (len == size - pos || len >= MAX_NAME_WSTR_SIZE) return ValidationResult::NRSSR_NameString_Overflow;
		pos += 2 * (len + 1);

		// Check host online id and session data size
		if (size - pos != sizeof(SESSION_DATA_SIZE) + HOST_ONLINE_ID_SIZE + SESSION_DATA_SIZE) 
			return ValidationResult::NRSSR_RemainingDataSize_Mismatch;
		
		// One single big endian number when everything else is little endian, this is weird
		uint16_t data_size = (uint16_t)nrssr_data[pos + HOST_ONLINE_ID_SIZE] << 8 | (uint16_t)nrssr_data[pos + HOST_ONLINE_ID_SIZE + 1];
		return (data_size == SESSION_DATA_SIZE) ? ValidationResult::Valid : ValidationResult::NRSSR_SessionSize_Abnormal;
	}
};