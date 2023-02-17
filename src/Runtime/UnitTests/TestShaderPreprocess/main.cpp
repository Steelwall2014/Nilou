#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
namespace GameStatics {
    
    bool StartsWith(const std::string &str, const std::string &temp)
    {
        if (str.size() < temp.size())
            return false;
        int length = std::min(str.size(), temp.size());
        for (int i = 0; i < length; i++)
        {
            if (str[i] != temp[i])
                return false;
        }
        return true;
    }
}
namespace nilou {
class AssetLoader 
{
public:
	std::string SyncOpenAndReadText(const char *filePath)
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
    class FShaderParameterCode
    {
    public:
        std::string Name;
        std::string Code;
        bool operator<(const FShaderParameterCode &Other) const
        {
            return Name < Other.Name;
        }
        bool operator==(const FShaderParameterCode &Other) const
        {
            return Name == Other.Name;
        }
    };

    struct FShaderTypeBase
    {
        std::string Name;
        std::string VirtualFilePath;
        std::string SourceCodeBody;
        std::filesystem::path FileAbsolutePath;
        std::set<FShaderParameterCode> ShaderParameterCodes;
        
        FShaderTypeBase::FShaderTypeBase() { }

        /** 
        Some preprocess operations will be done in the constructor, 
        like change the relative include path to absolute path and discard the #version line
        */
        FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InFileName);
    };

}
namespace fs = std::filesystem;

namespace nilou {

    std::string ProcessIncludePath(const std::string &RawSourceCode, const std::filesystem::path &FileAbsolutePath)
    {
        std::smatch matches;
        std::stringstream Input(RawSourceCode);
        std::stringstream Output;
        std::string lineBuffer;
        std::regex re_version("^[ ]*#[ ]*version[ ]+[0-9]+.*");
        std::regex re_include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
        while (std::getline(Input, lineBuffer))
        {
            if (std::regex_match(lineBuffer, matches, re_version))
            {
                continue;
            }
            else if (std::regex_match(lineBuffer, matches, re_include))
            {
                std::string included_path_str = matches[1];
                fs::path included_path = fs::path(included_path_str);
                if (included_path.is_absolute())
                {
                    Output << lineBuffer << "\n";
                    continue;
                }
                fs::path FileParentPath = FileAbsolutePath.parent_path();
                while (GameStatics::StartsWith(included_path_str, "../") || GameStatics::StartsWith(included_path_str, "..\\"))
                {
                    included_path_str = included_path_str.substr(3);
                    FileParentPath = FileParentPath.parent_path();
                }
                included_path = FileParentPath / fs::path(included_path_str);
                Output << "#include \"" << included_path.generic_string() << "\"\n";
            }
            else 
            {
                Output << lineBuffer << "\n";
            }
        }
        return Output.str();
    }

    std::string ProcessIncludeInternal(const std::string &RawSourceCode, std::set<std::string> &AlreadyIncludedPathes)
    {
        std::stringstream input(RawSourceCode);
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
                    std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(absolute_path.generic_string().c_str());
                    AlreadyIncludedPathes.insert(absolute_path.generic_string());
                    fullSourceCode << ProcessIncludeInternal(SourceCode, AlreadyIncludedPathes);
                }
            }
            else 
            {
                fullSourceCode << lineBuffer + '\n';
            }
        }

        return fullSourceCode.str();
    }

    std::string ProcessInclude(const std::string &RawSourceCode)
    {
        std::set<std::string> AlreadyIncludedPathes;
        return ProcessIncludeInternal(RawSourceCode, AlreadyIncludedPathes);
    }


    FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InVirtualFilePath)
        : Name(InClassName)
        , VirtualFilePath(InVirtualFilePath)
    {
        // for (const fs::path &WorkDir : AssetLoader::WorkDirectory)
        // {
        //     FileAbsolutePath = AssetLoader::FileExistsInDir(WorkDir, InFileName);
        //     if (!FileAbsolutePath.empty())
        //         break;
        // }
        if (InVirtualFilePath != "")
        {
            std::smatch matches;
            std::string RawSourceCode = GetAssetLoader()->SyncOpenAndReadText(VirtualFilePath.c_str());

            RawSourceCode = ProcessIncludePath(RawSourceCode, VirtualFilePath);

            RawSourceCode = ProcessInclude(RawSourceCode);

            std::regex re_uniformbuffer("^[ ]*(layout[ ]*\\(.*\\)[ ]*uniform[ ]+)([a-zA-Z_]+\\w*)[ \\n]*\\{([ \\w\\n;]+)\\}(.*;)[ \\n]*");
            std::regex re_binding1(",[ ]*binding[ ]*=[ ]*[0-9]+[ ]*");              // like layout(std140, binding = 0)
            std::regex re_binding2("[ ]*binding[ ]*=[ ]*[0-9]+,[ ]*");              // like layout(binding = 0, std140)
            std::regex re_binding3("\\([ ]*binding[ ]*=[ ]*[0-9]+[ ]*\\)");         // like layout(binding = 0)

            std::string temp = RawSourceCode;
            while (std::regex_search(temp, matches, re_uniformbuffer))
            {
                std::string prefix = matches[1];
                std::string buffer_name = matches[2];
                std::string body = matches[3];
                std::string suffix = matches[4];
                prefix = std::regex_replace(prefix, re_binding1, "");
                prefix = std::regex_replace(prefix, re_binding2, "");
                prefix = std::regex_replace(prefix, re_binding3, "(std140)");
                FShaderParameterCode Code;
                Code.Name = buffer_name;
                Code.Code = prefix + " " + buffer_name + " {" + body + "}" + suffix;
                ShaderParameterCodes.insert(Code);
                temp = matches.suffix();
            }

            // like layout(...) uniform sampler2D tex;
            std::regex re_sampler("^[ ]*layout[ ]*\\(.*\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
            // like uniform sampler2D tex; / uniform int a;
            std::regex re_uniform("[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");

            temp = RawSourceCode;
            while (std::regex_search(temp, matches, re_uniform))
            {
                std::string parameter_type = matches[1];
                std::string buffer_name = matches[2];
                FShaderParameterCode Code;
                Code.Name = buffer_name;
                Code.Code = "uniform " + parameter_type + " " + buffer_name + ";\n";
                ShaderParameterCodes.insert(Code);
                temp = matches.suffix();
            }
            RawSourceCode = std::regex_replace(RawSourceCode, re_uniformbuffer, "");
            RawSourceCode = std::regex_replace(RawSourceCode, re_sampler, "");
            RawSourceCode = std::regex_replace(RawSourceCode, re_uniform, "");

            SourceCodeBody = RawSourceCode;
        }

    }
}



int main()
{
    nilou::FShaderTypeBase type("", "D:\\Nilou\\Assets\\Shaders\\shader_preprocess_test.frag");
    std::ofstream out("D:\\Nilou\\src\\Runtime\\UnitTests\\TestShaderPreprocess\\res.frag");
    for (auto &Code : type.ShaderParameterCodes)
    {
        out << Code.Code << "\n";
    }
    out << type.SourceCodeBody;
}