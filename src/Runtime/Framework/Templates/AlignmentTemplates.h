#pragma once
#include <type_traits>
#include "Platform.h"

namespace nilou {

/**
 * Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.
 *
 * @param  Val        The value to align.
 * @param  Alignment  The alignment value, must be a power of two.
 *
 * @return The value aligned up to the specified alignment.
 */
template <typename T>
inline constexpr T Align(T Val, uint64 Alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "Align expects an integer or pointer type");

	return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
}

}