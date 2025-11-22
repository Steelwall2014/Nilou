#pragma once
#include <string>
#include "HAL/Platform.h"

namespace nilou {

struct CORE_API FFileHelper
{
    static bool LoadFileToString(std::string& OutString, const std::string& Filename);
    static bool SaveStringToFile(const std::string& String, const std::string& Filename);
};

}