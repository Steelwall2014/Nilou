#include "Misc/Paths.h"

namespace fs = std::filesystem;

namespace nilou {

std::string FPaths::GetBaseFilename(const std::string &InPath)
{
    return std::filesystem::path(InPath).filename().generic_string();
}

std::string FPaths::EngineDir()
{
    static std::string EngineDirectory = "../../../../Engine";
    return EngineDirectory;
}

std::string FPaths::EngineContentDir()
{
    static std::string ContentDirectory = FPaths::EngineDir() + "/Content";
    return ContentDirectory;
}

std::string FPaths::EngineSavedDir()
{
    static std::string SavedDirectory = FPaths::EngineDir() + "/Saved";
    return SavedDirectory;
}

bool FPackageName::DoesPackageExist(const std::string& PackageName)
{
    return fs::exists(FPackageName::LongPackageNameToMetaFileName(PackageName));
}

}