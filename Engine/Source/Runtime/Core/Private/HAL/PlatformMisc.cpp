#include <windows.h>
#include "HAL/PlatformMisc.h"
#include "Math/Maths.h"

namespace nilou {

void FWindowsPlatformMisc::BeginNamedEvent(const FColor& Color, const std::string& Text)
{
    // Not implemented
    // Steelwall2014: TODO: Implement this
}

void FWindowsPlatformMisc::EndNamedEvent()
{
    // Not implemented
    // Steelwall2014: TODO: Implement this
}

bool FWindowsPlatformMisc::IsDebuggerPresent()
{
    return ::IsDebuggerPresent();
}

}