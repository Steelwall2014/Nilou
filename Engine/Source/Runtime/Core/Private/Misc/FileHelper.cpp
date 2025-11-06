#include "Misc/FileHelper.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
namespace nilou {

bool FFileHelper::LoadFileToString(std::string& OutString, const std::string& Filename)
{
    fs::path path = fs::path(Filename);
    if (!fs::exists(path))
    {
        return false;
    }
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    OutString = ss.str();
    return true;
}

bool FFileHelper::SaveStringToFile(const std::string& String, const std::string& Filename)
{
    fs::path path = fs::path(Filename);
    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    file << String;
    return true;
}

}