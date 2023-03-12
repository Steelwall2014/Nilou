#include "Path.h"

namespace nilou {

    std::filesystem::path FPath::ProjectDir()
    {
        static std::filesystem::path ProjectDirectory = "D:\\Nilou";
        return ProjectDirectory;
    }

    std::filesystem::path FPath::ShaderDir()
    {
        static std::filesystem::path ShaderDirectory = "D:\\Nilou\\Content\\Shaders";
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

    std::filesystem::path FPath::ContentDir()
    {
        static std::filesystem::path ContentDirectory = "D:\\Nilou\\Content";
        return ContentDirectory;
    }

    std::string FPath::RelativePath(const std::string &from, const std::string &to)
    {
        std::string result;
        int i,j,k;
        i = j = 0;
        while(from[i] != '\0' && to[i] != '\0' && from[i] == to[i]){
            i++;
        }
        k = i;
        while(from[k] != '\0'){
            if(from[k++] == '/'){
                result.push_back('.');
                result.push_back('.');
                result.push_back('/');
            }else{
                continue;
            }
        }
        while(to[i] != '\0'){
            result.push_back(to[i++]);
        }
        return result;
    }

}