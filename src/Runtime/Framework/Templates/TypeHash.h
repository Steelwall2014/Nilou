#pragma once
#include "Platform.h"
#include "Common/AssertionMacros.h"

namespace nilou {

uint32 HashCombineFast(uint32 A, uint32 B);

/**
 * Combines two hash values to get a third.
 * Note - this function is not commutative.
 *
 * This function cannot change for backward compatibility reasons.
 * You may want to choose HashCombineFast for a better in-memory hash combining function.
 * 
 * Steelwall2014: We just use HashCombineFast because we don't need to consider backward compatibility reasons.
 * 
 */
inline uint32 HashCombine(uint32 A, uint32 C)
{
    return HashCombineFast(A, C);
}
/*inline uint32 HashCombine(uint32 A, uint32 C)
{
	uint32 B = 0x9e3779b9;
	A += B;

	A -= B; A -= C; A ^= (C>>13);
	B -= C; B -= A; B ^= (A<<8);
	C -= A; C -= B; C ^= (B>>13);
	A -= B; A -= C; A ^= (C>>12);
	B -= C; B -= A; B ^= (A<<16);
	C -= A; C -= B; C ^= (B>>5);
	A -= B; A -= C; A ^= (C>>3);
	B -= C; B -= A; B ^= (A<<10);
	C -= A; C -= B; C ^= (B>>15);

	return C;
}*/

/**
 * Combines two hash values to get a third.
 * Note - this function is not commutative.
 *
 * WARNING!  This function is subject to change and should only be used for creating
 *           combined hash values which don't leave the running process,
 *           e.g. GetTypeHash() overloads.
 */
inline uint32 HashCombineFast(uint32 A, uint32 B)
{
	return A ^ (B + 0x9e3779b9 + (A << 6) + (A >> 2));
}

FORCEINLINE uint32 GetTypeHash(uint32 Value)
{
	return std::hash<uint32>()(Value);
}

FORCEINLINE int32 GetTypeHash(int32 Value)
{
	return std::hash<int32>()(Value);
}

FORCEINLINE uint64 GetTypeHash(uint64 Value)
{
	return std::hash<uint64>()(Value);
}

}
