// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <magic_enum/magic_enum.hpp>
#include <string>
#include <string_view>
#include <type_traits>

inline void AppendEnumFlagName(std::string& Result, std::string_view Name)
{
	if (!Result.empty())
	{
		Result += " | ";
	}
	Result += Name;
}

template<typename Enum>
constexpr bool IsSingleBitEnumValue(std::make_unsigned_t<__underlying_type(Enum)> Value)
{
	return Value != 0 && (Value & (Value - 1)) == 0;
}

template<typename Enum>
inline std::string EnumFlagsToString(Enum Flags)
{
	using UnderlyingType = __underlying_type(Enum);
	using UnsignedType = std::make_unsigned_t<UnderlyingType>;

	const UnsignedType Value = static_cast<UnsignedType>(static_cast<UnderlyingType>(Flags));
	if (Value == 0)
	{
		return "None";
	}

	UnsignedType Remaining = Value;
	std::string Result;
	for (Enum EnumValue : magic_enum::enum_values<Enum>())
	{
		const UnsignedType FlagValue = static_cast<UnsignedType>(static_cast<UnderlyingType>(EnumValue));
		if (!IsSingleBitEnumValue<Enum>(FlagValue))
		{
			continue;
		}
		if ((Value & FlagValue) != 0)
		{
			AppendEnumFlagName(Result, magic_enum::enum_name(EnumValue));
			Remaining &= ~FlagValue;
		}
	}

	if (Remaining != 0)
	{
		const std::string Unknown = "Unknown(" + std::to_string(static_cast<unsigned long long>(Remaining)) + ")";
		AppendEnumFlagName(Result, Unknown);
	}
	return Result.empty() ? "None" : Result;
}

// Defines all bitwise operators for enum classes so it can be (mostly) used as a regular flags enum
#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; } \
	inline std::string     ToString(Enum Flags)            { return EnumFlagsToString(Flags); }

// Friends all bitwise operators for enum classes so the definition can be kept private / protected.
#define FRIEND_ENUM_CLASS_FLAGS(Enum) \
	friend           Enum& operator|=(Enum& Lhs, Enum Rhs); \
	friend           Enum& operator&=(Enum& Lhs, Enum Rhs); \
	friend           Enum& operator^=(Enum& Lhs, Enum Rhs); \
	friend constexpr Enum  operator| (Enum  Lhs, Enum Rhs); \
	friend constexpr Enum  operator& (Enum  Lhs, Enum Rhs); \
	friend constexpr Enum  operator^ (Enum  Lhs, Enum Rhs); \
	friend constexpr bool  operator! (Enum  E); \
	friend constexpr Enum  operator~ (Enum  E);

template<typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
	return ( ( ( __underlying_type(Enum) )Flags ) & ( __underlying_type(Enum) )Contains ) == ( ( __underlying_type(Enum) )Contains );
}

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	return ( ( ( __underlying_type(Enum) )Flags ) & ( __underlying_type(Enum) )Contains ) != 0;
}

template<typename Enum>
void EnumAddFlags(Enum& Flags, Enum FlagsToAdd)
{
	Flags |= FlagsToAdd;
}

template<typename Enum>
void EnumRemoveFlags(Enum& Flags, Enum FlagsToRemove)
{
	Flags &= ~FlagsToRemove;
}