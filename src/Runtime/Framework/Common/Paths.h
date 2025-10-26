#pragma once

#include <filesystem>

namespace nilou {

class FPaths
{
public:
    static std::string GetBaseFilename(const std::string &InPath);
};

}