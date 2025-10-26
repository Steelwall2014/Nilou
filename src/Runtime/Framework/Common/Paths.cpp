#include "Paths.h"

namespace nilou {

std::string FPaths::GetBaseFilename(const std::string &InPath)
{
    return std::filesystem::path(InPath).filename().generic_string();
}

}