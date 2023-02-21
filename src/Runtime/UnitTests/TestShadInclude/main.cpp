#include "GameStatics.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

namespace nilou {
class AssetLoader
{
public:
	std::string AssetLoader::SyncOpenAndReadText(const char *filePath)
	{
		std::filesystem::path absolute_path;
		std::filesystem::path FilePath = std::filesystem::path(filePath);

			if (std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath))
				absolute_path = FilePath;



		if (absolute_path.empty())
			return "";

        std::stringstream res;
        std::ifstream stream(absolute_path);
        char buffer[1024];
        while (stream.getline(buffer, sizeof(buffer)))
        {
            res << buffer << "\n";
        }
		return res.str();
		// FILE *fp = fopen(absolute_path.c_str(), "r");
		// long pos = ftell(fp);
		// fseek(fp, 0, SEEK_END);
		// size_t length = ftell(fp);
		// fseek(fp, pos, SEEK_SET);
		// char *data = new char[length + 1];
		// length = fread(data, 1, length, fp);
		// data[length] = '\0';
		// fclose(fp);
		// return data;
	}

};
AssetLoader *GetAssetLoader() = new AssetLoader;
}

class Shadinclude
{
public:

	static std::string Preprocess(const std::string &RawCode, const std::vector<std::filesystem::path> &IncludeDir, std::string includeIndentifier = "#include");

	// Return the source code of the complete shader
	static std::string load(std::string path, std::string includeIndentifier = "#include");

private:
	static void getFilePath(const std::string & fullPath, std::string & pathWithoutFileName)
	{
		// Remove the file name and store the path to this folder
		size_t found = fullPath.find_last_of("/\\");
		pathWithoutFileName = fullPath.substr(0, found + 1);
	}

	static void getFileParentDirPath(const std::string &fullPath, std::string &parentPath)
	{
		// Remove the file name and store the path to this folder
		size_t found = fullPath.find_last_of("/\\");
		std::string pathWithoutFileName = fullPath.substr(0, found);
		found = pathWithoutFileName.find_last_of("/\\");
		parentPath = pathWithoutFileName.substr(0, found + 1);
	}

	// static bool startswith(const std::string &str, const std::string &temp)
	// {
	// 	if (str.size() < temp.size())
	// 		return false;
	// 	int length = std::min(str.size(), temp.size());
	// 	for (int i = 0; i < length; i++)
	// 	{
	// 		if (str[i] != temp[i])
	// 			return false;
	// 	}
	// 	return true;
	// }

};


std::string Shadinclude::Preprocess(const std::string &RawCode, const std::vector<std::filesystem::path> &IncludeDir, std::string includeIndentifier)
{
    includeIndentifier += ' ';
    static bool isRecursiveCall = false;

    // std::string fullSourceCode = "";
    std::stringstream input(RawCode);
    std::stringstream fullSourceCode;

    std::string lineBuffer;
    std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    while (std::getline(input, lineBuffer))
    {
        // Look for the new shader include identifier
        std::smatch matches;
        if (std::regex_match(lineBuffer, matches, re))
        {
            // Remove the include identifier, this will cause the path to remain
            std::string include_file = matches[1];
            std::filesystem::path included_path(include_file);
            std::filesystem::path filename = included_path.filename();
            std::filesystem::path absolute_path;

            for (auto &include_dir : IncludeDir)
            {
                absolute_path = include_dir / included_path;
                if (std::filesystem::exists(absolute_path))
                    break;
            }

            /** 直接在AssetLoader::ShaderDirectories里找文件存不存在，不管路径 */
            // for (auto &shader_dir : nilou::AssetLoader::ShaderDirectories)
            // {
            //     absolute_path = nilou::AssetLoader::FileExistsInDir(shader_dir, filename);
            //     if (!absolute_path.empty())
            //         break;
            // }
            // if (GameStatics::StartsWith(lineBuffer, "../"))
            // {
            //     lineBuffer = lineBuffer.substr(3);
            //     lineBuffer = CodeDir.parent_path().generic_string() + lineBuffer;
            // }
            // else
            // {
            //     // The include path is relative to the current shader file path
            //     lineBuffer = CodeDir.generic_string() + lineBuffer;
            // }

            // By using recursion, the new include file can be extracted
            // and inserted at this location in the shader source code
            isRecursiveCall = true;
            std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(absolute_path.generic_string().c_str());
            fullSourceCode << Preprocess(SourceCode, IncludeDir);// load(absolute_path.generic_string());

            // Do not add this line to the shader source code, as the include
            // path would generate a compilation issue in the final source code
            continue;
        }

        fullSourceCode << lineBuffer + '\n';
    }

    // Only add the null terminator at the end of the complete file,
    // essentially skipping recursive function calls this way
    if (!isRecursiveCall)
        fullSourceCode << '\0';


    return fullSourceCode.str();
}

std::string Shadinclude::load(std::string path, std::string includeIndentifier)
{
    includeIndentifier += ' ';
    static bool isRecursiveCall = false;

    std::string fullSourceCode = "";
    std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(path.c_str());
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


int main()
{
    // std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText("D:\\UnderwaterRendering\\Nilou\\Assets\\Shaders\\waterbody\\include_test.glsl");
    // std::string res = Shadinclude::Preprocess(SourceCode, {"D:\\UnderwaterRendering\\Nilou\\Assets\\Shaders"});
    // std::filesystem::path FileAbsolutePath("D:\\UnderwaterRendering\\Nilou\\Assets\\Shaders\\waterbody\\include_test.glsl");
    // std::stringstream Input(nilou::GetAssetLoader()->SyncOpenAndReadText(FileAbsolutePath.generic_string().c_str()));
    // std::stringstream Output;
    // std::string lineBuffer;
    // std::regex re_version("^[ ]*#[ ]*version[ ]+[0-9].*");
    // std::regex re_include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
    // while (std::getline(Input, lineBuffer))
    // {
    //     std::smatch matches;
    //     if (std::regex_match(lineBuffer, matches, re_version))
    //     {
    //         continue;
    //     }
    //     else if (std::regex_match(lineBuffer, matches, re_include))
    //     {
    //         std::string included_path_str = matches[1];
    //         // fs::path included_path = fs::path(included_path_str);
    //         std::filesystem::path FileParentPath = FileAbsolutePath.parent_path();
    //         while (GameStatics::StartsWith(included_path_str, "../") || GameStatics::StartsWith(included_path_str, "..\\"))
    //         {
    //             included_path_str = included_path_str.substr(3);
    //             FileParentPath = FileParentPath.parent_path();
    //         }
    //         std::filesystem::path included_path = FileParentPath / std::filesystem::path(included_path_str);
    //         Output << "#include \"" << included_path.generic_string() << "\"\n";
    //     }
    //     else 
    //     {
    //         Output << lineBuffer << "\n";
    //     }
    // }
    // std::string SourceCodeBody = Output.str();
    std::filesystem::path p("a\\b");
    std::string a = p.generic_string();
}