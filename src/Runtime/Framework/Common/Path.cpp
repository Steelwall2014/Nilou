#include "Path.h"

namespace nilou {

    std::filesystem::path FPath::ProjectDir()
    {
        static std::filesystem::path ProjectDirectory = "D:\\Nilou";
        return ProjectDirectory;
    }

    std::filesystem::path FPath::ShaderDir()
    {
        static std::filesystem::path ShaderDirectory = "D:\\Nilou\\Assets\\Shaders";
        return ShaderDirectory;
    }

    std::filesystem::path FPath::AssetsDir()
    {
        static std::filesystem::path AssetsDirectory = "D:\\Nilou\\Assets";
        return AssetsDirectory;
    }

    std::filesystem::path FPath::LaunchDir()
    {
        static std::filesystem::path LaunchDirectory = "D:\\Nilou\\build\\windows\\x64\\debug";
        return LaunchDirectory;
    }

    std::filesystem::path FPath::MaterialDir()
    {
        static std::filesystem::path MaterialDirectory = ShaderDir() / "Materials";
        return MaterialDirectory;
    }

}