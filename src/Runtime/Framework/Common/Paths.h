#pragma once

#include <filesystem>

namespace nilou {

class FPaths
{
public:
    static std::string GetBaseFilename(const std::string &InPath);
    static std::string ContentDir();
    static std::string ProjectDir();
};

class FPackageName
{
public:
    // PackageName should start with '/'
    static std::string LongPackageNameToMetaFileName(const std::string& PackageName)
    {
        return (FPaths::ContentDir() / std::filesystem::path(PackageName.substr(1) + ".nasset.meta")).generic_string();
    }

    // PackageName should start with '/'
    static std::string LongPackageNameToFileName(const std::string& PackageName)
    {
        return (FPaths::ContentDir() / std::filesystem::path(PackageName.substr(1) + ".nasset")).generic_string();
    }

    static bool DoesPackageExist(const std::string& PackageName);

};

}