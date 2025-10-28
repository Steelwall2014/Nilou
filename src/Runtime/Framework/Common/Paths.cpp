#include "Paths.h"

namespace fs = std::filesystem;

namespace nilou {

std::string FPaths::GetBaseFilename(const std::string &InPath)
{
    return std::filesystem::path(InPath).filename().generic_string();
}

std::string FPaths::ContentDir()
{
    static std::string ContentDirectory = FPaths::ProjectDir() + "/Content";
    return ContentDirectory;
}

std::string FPaths::ProjectDir()
{
    static std::string ProjectDirectory = PROJECT_DIR;
    return ProjectDirectory;
}

bool FPackageName::DoesPackageExist(const std::string& PackageName)
{
    return fs::exists(FPackageName::LongPackageNameToMetaFileName(PackageName));
}

}