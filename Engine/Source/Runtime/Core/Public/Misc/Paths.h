#pragma once

#include <filesystem>
#include "HAL/Platform.h"

namespace nilou {

class CORE_API FPaths
{
public:
    static std::string GetBaseFilename(const std::string &InPath);
    static std::string EngineDir();
    static std::string EngineContentDir();
    static std::string EngineSavedDir();
    static std::string EngineShadersPublicDir();
    static std::string VirtualPathToAbsPath(const std::string &VirtualPath);
};

class CORE_API FPackageName
{
public:
    // PackageName should start with '/'
    static std::string LongPackageNameToMetaFileName(const std::string& PackageName)
    {
        if (PackageName.starts_with("/Engine"))
        {
            std::string LocalPackageName = PackageName.substr(7);   // remove "/Engine"
            std::string Path = FPaths::EngineDir() + "/Content" + LocalPackageName + ".nasset.meta";
            return Path;
        }
        return "";
    }

    // PackageName should start with '/'
    static std::string LongPackageNameToFileName(const std::string& PackageName)
    {
        if (PackageName.starts_with("/Engine"))
        {
            std::string LocalPackageName = PackageName.substr(7);   // remove "/Engine"
            std::string Path = FPaths::EngineDir() + "/Content" + LocalPackageName + ".nasset";
            return Path;
        }
        return "";
    }

    static bool DoesPackageExist(const std::string& PackageName);

};

}