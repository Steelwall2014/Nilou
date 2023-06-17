#pragma once
#include <type_traits>
#include "Platform.h"

/** 
 * CRC hash generation for different types of input data
 * From Unreal Engine 5.0 Engine\Source\Runtime\Core\Public\Misc\Crc.h
 **/
struct FCrc
{
	/** lookup table with precalculated CRC values - slicing by 8 implementation */
	static uint32 CRCTablesSB8[8][256];

	/** initializes the CRC lookup table. Must be called before any of the
		CRC functions are used. */
	static void Init();

	/** generates CRC hash of the memory area */
	static uint32 MemCrc32( const void* Data, int32 Length, uint32 CRC=0 );

	/** generates CRC hash of the element */
	template <typename T>
	static uint32 TypeCrc32( const T& Data, uint32 CRC=0 )
	{
		return MemCrc32(&Data, sizeof(T), CRC);
	}

	/** String CRC. */
	template <typename CharType>
	static typename std::enable_if<sizeof(CharType) != 1, uint32>::Type StrCrc32(const CharType* Data, uint32 CRC = 0)
	{
		// We ensure that we never try to do a StrCrc32 with a CharType of more than 4 bytes.  This is because
		// we always want to treat every CRC as if it was based on 4 byte chars, even if it's less, because we
		// want consistency between equivalent strings with different character types.
		static_assert(sizeof(CharType) <= 4, "StrCrc32 only works with CharType up to 32 bits.");

		CRC = ~CRC;
		while (CharType Ch = *Data++)
		{
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			Ch >>= 8;
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			Ch >>= 8;
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			Ch >>= 8;
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
		}
		return ~CRC;
	}

	template <typename CharType>
	static typename std::enable_if<sizeof(CharType) == 1, uint32>::Type StrCrc32(const CharType* Data, uint32 CRC = 0)
	{
		/* Overload for when CharType is a byte, which causes warnings when right-shifting by 8 */
		CRC = ~CRC;
		while (CharType Ch = *Data++)
		{
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
			CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
		}
		return ~CRC;
	}

};