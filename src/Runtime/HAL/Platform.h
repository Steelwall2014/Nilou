#include <cstdint>
// Put it here to always include Ncheck. (Platform.h is included by almost every file)
// Ncheck is IMPORTANT! Use it frequently!
// For example, bounds checking
#include "Common/Log.h" 

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

#define FORCEINLINE __forceinline