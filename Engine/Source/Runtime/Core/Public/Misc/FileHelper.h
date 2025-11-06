#pragma once
#include <string>

namespace nilou {

struct FFileHelper
{
    static bool LoadFileToString(std::string& OutString, const std::string& Filename);
    static bool SaveStringToFile(const std::string& String, const std::string& Filename);
};

}