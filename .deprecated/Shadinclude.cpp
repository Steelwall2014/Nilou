#include "Shadinclude.h"
#include "Common/AssetLoader.h"
#include "GameStatics.h"
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

std::string Shadinclude::Preprocess(const std::string &RawCode)
{
    std::set<std::string> AlreadyIncludedPathes;
    PreprocessInternal(RawCode, AlreadyIncludedPathes);
}

std::string Shadinclude::PreprocessInternal(const std::string &RawCode, std::set<std::string> &AlreadyIncludedPathes)
{
    std::stringstream input(RawCode);
    std::stringstream fullSourceCode;

    std::string lineBuffer;
    std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    while (std::getline(input, lineBuffer))
    {
        std::smatch matches;
        if (std::regex_match(lineBuffer, matches, re))
        {
            /** The include path has been preprocessed in FShaderTypeBase construct function so it's now an absolute path */
            std::filesystem::path absolute_path = std::filesystem::path(std::string(matches[1]));
            if (AlreadyIncludedPathes.find(absolute_path.generic_string()) == AlreadyIncludedPathes.end())
            {
                std::string SourceCode = und::g_pAssetLoader->SyncOpenAndReadText(absolute_path.generic_string().c_str());
                AlreadyIncludedPathes.insert(absolute_path.generic_string());
                fullSourceCode << PreprocessInternal(SourceCode, AlreadyIncludedPathes);
            }
        }
        else 
        {
            fullSourceCode << lineBuffer + '\n';
        }
    }

    return fullSourceCode.str();
}

std::string Shadinclude::load(std::string path, std::string includeIndentifier)
{
    includeIndentifier += ' ';
    static bool isRecursiveCall = false;

    std::string fullSourceCode = "";
    std::string SourceCode = und::g_pAssetLoader->SyncOpenAndReadText(path.c_str());
    std::vector<std::string> Lines = GameStatics::Split(SourceCode, '\n');
    // std::ifstream file(path);

    // if (!file.is_open())
    // {
    // 	std::cerr << "ERROR: could not open the shader at: " << path << "\n" << std::endl;
    // 	return fullSourceCode;
    // }
    // std::string lineBuffer;
    for (std::string &lineBuffer : Lines)
    {
        // Look for the new shader include identifier
        if (lineBuffer.find(includeIndentifier) != lineBuffer.npos && !GameStatics::StartsWith(lineBuffer, "//"))
        {
            // Remove the include identifier, this will cause the path to remain
            lineBuffer.erase(0, includeIndentifier.size());
            lineBuffer = lineBuffer.substr(1, lineBuffer.size()-2);
            if (GameStatics::StartsWith(lineBuffer, "../"))
            {
                lineBuffer = lineBuffer.substr(3, lineBuffer.size() - 3);
                std::string parentPath;
                getFileParentDirPath(path, parentPath);
                lineBuffer.insert(0, parentPath);
            }
            else
            {
                // The include path is relative to the current shader file path
                std::string pathOfThisFile;
                getFilePath(path, pathOfThisFile);
                lineBuffer.insert(0, pathOfThisFile);
            }

            // By using recursion, the new include file can be extracted
            // and inserted at this location in the shader source code
            isRecursiveCall = true;
            fullSourceCode += load(lineBuffer);

            // Do not add this line to the shader source code, as the include
            // path would generate a compilation issue in the final source code
            continue;
        }

        fullSourceCode += lineBuffer + '\n';
    }

    // Only add the null terminator at the end of the complete file,
    // essentially skipping recursive function calls this way
    if (!isRecursiveCall)
        fullSourceCode += '\0';

    return fullSourceCode;
}