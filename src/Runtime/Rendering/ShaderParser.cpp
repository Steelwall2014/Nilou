#include <regex>
#include <sstream>
#include <vector>

#include "ShaderParser.h"
#include "GameStatics.h"
#include "DynamicRHI.h"
#include "Common/AssetLoader.h"
#include "Common/Log.h"


namespace fs = std::filesystem;

namespace nilou {


    FShaderParserResult FShaderParser::Parse()
    {
        Code = ParseInclude();
        // NILOU_LOG(Info, Code)
        Code = ParseParameters();
        // NILOU_LOG(Info, Code)
        ParsedResult.MainCode = Code;
        return ParsedResult;
    }

    std::filesystem::path ShaderIncludePathToAbsolute(const std::string &IncludePathStr, const std::filesystem::path &FileParentDir)
    {
        std::string included_path_str = IncludePathStr;
        fs::path included_path = fs::path(included_path_str);
        if (included_path.is_absolute())
        {
            return included_path;
        }
        std::filesystem::path LocalFileParentDir = FileParentDir;
        while (GameStatics::StartsWith(included_path_str, "../") || GameStatics::StartsWith(included_path_str, "..\\"))
        {
            included_path_str = included_path_str.substr(3);
            LocalFileParentDir = LocalFileParentDir.parent_path();
        }
        return LocalFileParentDir / fs::path(included_path_str);
    }

    std::string ParseIncludePath(const std::string &RawSourceCode, const std::filesystem::path &FileParentDir)
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
                std::filesystem::path LocalFileParentDir = FileParentDir;
                while (GameStatics::StartsWith(included_path_str, "../") || GameStatics::StartsWith(included_path_str, "..\\"))
                {
                    included_path_str = included_path_str.substr(3);
                    LocalFileParentDir = LocalFileParentDir.parent_path();
                }
                included_path = LocalFileParentDir / fs::path(included_path_str);
                Output << "#include \"" << included_path.generic_string() << "\"\n";
            }
            else 
            {
                Output << lineBuffer << "\n";
            }
        }
        return Output.str();
    }

    std::string ParseIncludeInternal(
        const std::string &RawSourceCode, 
        const std::filesystem::path &FileParentDir, 
        std::set<std::string> &AlreadyIncludedPathes)
    {
        std::stringstream input(RawSourceCode);
        std::stringstream fullSourceCode;

        std::string lineBuffer;
        std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
        std::regex re_version("^[ ]*#[ ]*version[ ]+[0-9]+.*");
        while (std::getline(input, lineBuffer))
        {
            std::smatch matches;
            if (std::regex_match(lineBuffer, matches, re_version))
            {
                continue;
            }
            if (std::regex_match(lineBuffer, matches, re))
            {
                std::filesystem::path absolute_path = ShaderIncludePathToAbsolute(matches[1].str(), FileParentDir);
                // std::filesystem::path absolute_path = std::filesystem::path(std::string(matches[1]));
                if (AlreadyIncludedPathes.find(absolute_path.generic_string()) == AlreadyIncludedPathes.end())
                {
                    std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(absolute_path.generic_string().c_str());
                    AlreadyIncludedPathes.insert(absolute_path.generic_string());
                    fullSourceCode << ParseIncludeInternal(SourceCode, absolute_path.parent_path(), AlreadyIncludedPathes);
                }
            }
            else 
            {
                fullSourceCode << lineBuffer + '\n';
            }
        }

        return fullSourceCode.str();
    }

    std::string FShaderParser::ParseInclude()
    {
        std::set<std::string> AlreadyIncludedPathes;
        return ParseIncludeInternal(Code, FileParentDir, AlreadyIncludedPathes);
    }

    std::string FShaderParser::ParseParameters()
    {
        std::stringstream Output;

        std::string temp = Code;
        std::smatch matches;
        std::regex re_binding1(",\\s*binding\\s*=\\s*[0-9]+\\s*");      // like layout(std140, binding = 0)
        std::regex re_binding2("\\s*binding\\s*=\\s*[0-9]+,\\s*");      // like layout(binding = 0, std140)
        std::regex re_binding3("\\s*binding\\s*=\\s*[0-9]+\\s*"); // like layout(binding = 0)
        std::regex re_layout("layout\\s*\\(\\s*\\)"); // like layout()
        std::regex re(R"(^\s*(layout\s+\(.*\)\s+uniform|uniform)\s+([a-zA-Z_]+\w*)\s*(\{([\s\S]*?)\}|[a-zA-Z_]+\w*)\s*;\s*$)");
        while (std::regex_search(temp, matches, re))
        {
            FShaderParsedParameter ParsedParameter;
            std::string prefix = matches[1].str();
            std::string parameter_type = matches[2].str();
            std::string parameter_name = matches[3].str();
            if (parameter_type == "image2D" || parameter_type == "image3D")
            {
                std::smatch binding_match;
                if (std::regex_search(prefix, binding_match, re_binding3))
                {
                    ParsedParameter.ParameterType = EShaderParameterType::SPT_Image;
                    ParsedParameter.Name = parameter_name;
                    ParsedParameter.Code = prefix + " " + parameter_type + " " + parameter_name + ";\n";
                }
                else 
                {
                    NILOU_LOG(Error, "image1/2/3D variables must have an explicit binding point");
                }
            }
            else if (parameter_type == "sampler2D" || parameter_type == "sampler3D" || parameter_type == "sampler2DArray")
            {
                prefix = std::regex_replace(prefix, re_binding1, "");
                prefix = std::regex_replace(prefix, re_binding2, "");
                prefix = std::regex_replace(prefix, re_binding3, "");
                prefix = std::regex_replace(prefix, re_layout, "");
                ParsedParameter.ParameterType = EShaderParameterType::SPT_Sampler;
                ParsedParameter.Name = parameter_name;
                ParsedParameter.Code = prefix + " " + parameter_type + " " + parameter_name + ";\n";
            }
            else if (std::regex_match(parameter_type, std::regex("(vec(2|3|4)|(d|b|i|u)vec(2|3|4)|mat(2|3|4)|float|double|int|uint|bool)")))
            {
                NILOU_LOG(Error, 
                    "All uniform variables must be inside a uniform block but {} is out of a uniform block",
                    parameter_name)
            }
            else 
            {
                prefix = std::regex_replace(prefix, re_binding1, "");
                prefix = std::regex_replace(prefix, re_binding2, "");
                prefix = std::regex_replace(prefix, re_binding3, "");
                prefix = std::regex_replace(prefix, re_layout, "(std140)");
                ParsedParameter.ParameterType = EShaderParameterType::SPT_UniformBuffer;
                parameter_name = matches[2].str();
                std::string parameter_body = matches[3].str();
                ParsedParameter.Name = parameter_name;
                ParsedParameter.Code = prefix + " " + parameter_type + " " + parameter_body + ";\n";
            }
            ParsedResult.ParsedParameters.push_back(ParsedParameter);
            Output << matches.prefix() << ParsedParameter.Code;
            temp = matches.suffix();
        }

        Output << temp;


        return Output.str();


    }
}