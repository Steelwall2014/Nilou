#pragma once
#include <string>
#include "Platform.h"

namespace nilou {

/**
* Windows implementation of the misc OS functions
**/
struct FWindowsPlatformMisc
{
    static void BeginNamedEvent(const struct FColor& Color, const std::string& Text);
    static void EndNamedEvent();
};
using FPlatformMisc = FWindowsPlatformMisc;

//
// Scoped named event class for constant (compile-time) strings literals.
//
// BeginNamedEventStatic works the same as BeginNamedEvent, but should only be passed a compile-time string literal.
// Some platform profilers can optimize the case where strings for certain events are constant.
//
class FScopedNamedEventStatic
{
public:

	FScopedNamedEventStatic(const struct FColor& Color, const std::string& Text)
	{
		FPlatformMisc::BeginNamedEvent(Color, Text);
	}

	~FScopedNamedEventStatic()
	{
		FPlatformMisc::EndNamedEvent();
	}
};

}
#define SCOPED_NAMED_EVENT(Name, Color) FScopedNamedEventStatic ScopedNamedEventStatic##__LINE__(Color, #Name)