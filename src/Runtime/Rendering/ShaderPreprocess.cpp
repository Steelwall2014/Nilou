#include <filesystem>
#include <regex>
#include <set>
#include "Common/Asset/AssetLoader.h"
#include "ShaderPreprocess.h"

namespace fs = std::filesystem;

namespace nilou {

namespace shader_preprocess {

static std::string ShaderIncludePathToAbsolute(
        const std::string &IncludePath, 
        const std::string& WorkingDirectory, 
        const std::vector<std::string>& IncludeDirectories)
{
    fs::path include_path = fs::path(IncludePath);
    if (include_path.is_absolute())
    {
        return include_path.generic_string();
    }
    fs::path absolute_path = fs::path(WorkingDirectory) / include_path;
    if (fs::exists(absolute_path))
    {
        return absolute_path.generic_string();
    }
    for (const std::string& IncludeDirectory : IncludeDirectories)
    {
        fs::path absolute_path = fs::path(IncludeDirectory) / include_path;
        if (fs::exists(absolute_path))
        {
            return absolute_path.generic_string();
        }
    }
    return "";
}

static std::string PreprocessIncludeInternal(
        const std::string& ShaderCode, 
        const std::string& WorkingDirectory,
        const std::vector<std::string>& IncludeDirectories,
        /** To avoid cyclic dependencies */
        std::set<std::string>& IncludedFiles)
{
    std::stringstream input(ShaderCode);
    std::stringstream output;

    std::string lineBuffer;
    std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    while (std::getline(input, lineBuffer))
    {
        std::smatch matches;
        if (std::regex_match(lineBuffer, matches, re))
        {
            std::filesystem::path absolute_path = ShaderIncludePathToAbsolute(matches[1].str(), WorkingDirectory, IncludeDirectories);
            if (IncludedFiles.find(absolute_path.generic_string()) == IncludedFiles.end())
            {
                std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(absolute_path.generic_string().c_str());
                IncludedFiles.insert(absolute_path.generic_string());
                output << PreprocessIncludeInternal(
                    SourceCode, 
                    absolute_path.parent_path().generic_string(), 
                    IncludeDirectories, 
                    IncludedFiles);
            }
        }
        else 
        {
            output << lineBuffer << '\n';
        }
    }

    return output.str();
}

std::string PreprocessInclude(const std::string& ShaderCode, const std::string& WorkingDirectory, const std::vector<std::string>& IncludeDirectories)
{
    std::set<std::string> IncludedFiles;
    return PreprocessIncludeInternal(ShaderCode, WorkingDirectory, IncludeDirectories, IncludedFiles);
}

} // namespace shader_preprocess

} // namespace nilou