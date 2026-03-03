#pragma once
#include <cstdint>

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

#define FORCEINLINE __forceinline

constexpr int32 INDEX_NONE = -1;

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

// Alignment.
#if defined(__clang__)
	#define GCC_PACK(n) __attribute__((packed,aligned(n)))
	#define GCC_ALIGN(n) __attribute__((aligned(n)))
	#if defined(_MSC_VER)
		#define MS_ALIGN(n) __declspec(align(n)) // With -fms-extensions, Clang will accept either alignment attribute
	#endif
#else
	#define MS_ALIGN(n) __declspec(align(n))
#endif

#ifndef GCC_PACK
	#define GCC_PACK(n)
#endif
#ifndef GCC_ALIGN
	#define GCC_ALIGN(n)
#endif
#ifndef MS_ALIGN
	#define MS_ALIGN(n)
#endif