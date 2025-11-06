#pragma once

#include <filesystem>

namespace nilou {
    class FPath 
    {
    public:
        static std::filesystem::path ProjectDir();
        static std::filesystem::path ProjectSavedDir();
        static std::filesystem::path ShaderDir();
        static std::filesystem::path AssetsDir();
        static std::filesystem::path LaunchDir();
        static std::filesystem::path MaterialDir();
        static std::filesystem::path ContentDir();
        static std::string RelativePath(const std::string &from, const std::string &to);
        static std::filesystem::path VirtualPathToAbsPath(const std::string &VirtualPath);
        static std::filesystem::path GetBaseFilename(const std::filesystem::path &InPath);
    };
}