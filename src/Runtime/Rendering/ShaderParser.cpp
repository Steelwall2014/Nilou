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
                    std::string SourceCode = nilou::g_pAssetLoader->SyncOpenAndReadText(absolute_path.generic_string().c_str());
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
        std::stringstream Output;;

        std::smatch uniform_block_matches;
        std::smatch uniform_matches;

        // like layout(...) uniform Name {
        //      mat4 matrix
        // };
        std::regex re_uniformblock("^[ \\n]*(layout[ ]*\\(.*\\)[ ]*(uniform|buffer)[ ]+)([a-zA-Z_]+\\w*)[\\s]*\\{([\\w\\s\\[\\];]+)\\}([.\\n]*;)[\\s]*");

        // like uniform sampler2D tex; / uniform int a;
        std::regex re_uniform("^[ \\n]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");

        std::regex re_binding1(",[ ]*binding[ ]*=[ ]*[0-9]+[ ]*");      // like layout(std140, binding = 0)
        std::regex re_binding2("[ ]*binding[ ]*=[ ]*[0-9]+,[ ]*");      // like layout(binding = 0, std140)
        std::regex re_binding3("\\([ ]*binding[ ]*=[ ]*[0-9]+[ ]*\\)"); // like layout(binding = 0)

        std::string temp = Code;
        while (true)
        {
            bool uniform_block_found = std::regex_search(temp, uniform_block_matches, re_uniformblock);
            bool uniform_found = std::regex_search(temp, uniform_matches, re_uniform);
            bool uniform_block_first;
            if (uniform_block_found && uniform_found)
            {
                if (uniform_block_matches.prefix().length() < uniform_matches.prefix().length())
                {
                    uniform_block_first = true;
                }
                else 
                {
                    uniform_block_first = false;
                }
            }
            else if (uniform_block_found)
            {
                uniform_block_first = true;
            }
            else if (uniform_found)
            {
                uniform_block_first = false;
            }
            else 
            {
                break;
            }

            if (uniform_block_first)
            {
                // NILOU_LOG(Info, "Uniform block/Shader storage buffer found: "+uniform_block_matches[0].str());
                std::string prefix = uniform_block_matches[1];
                std::string buffer_type = uniform_block_matches[2];
                std::string buffer_name = uniform_block_matches[3];
                std::string body = uniform_block_matches[4];
                std::string suffix = uniform_block_matches[5];
                
                if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                {      
                    prefix = std::regex_replace(prefix, re_binding1, "");
                    prefix = std::regex_replace(prefix, re_binding2, "");
                    prefix = std::regex_replace(prefix, re_binding3, "(std140)");     
                }
                else 
                {
                    std::smatch matches;
                    std::regex re_binding4("\\(.*binding.*\\)");
                    std::regex re_binding5("\\((.*)\\)");
                    prefix = std::regex_replace(prefix, re_binding1, "{binding}");
                    prefix = std::regex_replace(prefix, re_binding2, "{binding}");
                    prefix = std::regex_replace(prefix, re_binding3, "(std140, {binding})");

                    // filter out those qualifiers that are not "binding=N"
                    if (!std::regex_search(prefix, matches, re_binding4))
                    {
                        std::regex_search(prefix, matches, re_binding5);
                        std::string in_brackets = matches[1];
                        prefix = "layout(" + in_brackets + ", {binding}) uniform";
                    }
                }

                FShaderParsedParameter ParsedParameter;
                if (buffer_type == "uniform")
                    ParsedParameter.ParameterType = EShaderParameterType::SPT_UniformBuffer;
                else if (buffer_type == "buffer")
                    ParsedParameter.ParameterType = EShaderParameterType::SPT_ShaderStructureBuffer;
                ParsedParameter.Name = buffer_name;
                ParsedParameter.Code = prefix + " " + buffer_name + " {" + body + "}" + suffix;
                ParsedResult.ParsedParameters.push_back(ParsedParameter);
                Output << uniform_block_matches.prefix() << ParsedParameter.Code << "\n";
                temp = uniform_block_matches.suffix();

            }
            else
            {
                // NILOU_LOG(Info, "sampler found: "+uniform_matches[0].str());
                std::string parameter_type = uniform_matches[1];
                std::string buffer_name = uniform_matches[2];
                FShaderParsedParameter ParsedParameter;
                if (GameStatics::StartsWith(parameter_type, "sampler"))
                {
                    ParsedParameter.ParameterType = EShaderParameterType::SPT_Sampler;
                    ParsedParameter.Name = buffer_name;
                    if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                        ParsedParameter.Code = "uniform " + parameter_type + " " + buffer_name + ";\n";
                    else if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
                        ParsedParameter.Code = "layout({binding}) uniform " + parameter_type + " " + buffer_name + ";\n";
                    ParsedResult.ParsedParameters.push_back(ParsedParameter);
                    Output << uniform_matches.prefix() << ParsedParameter.Code;
                }
                else 
                {
                    NILOU_LOG(Error, 
                        "All uniform variables must be inside a uniform block but " 
                        + buffer_name + " is out of a uniform block")
                    // Code.ParameterType = EShaderParameterType::SPT_Uniform;
                }
                temp = uniform_matches.suffix();
            }
        }

        Output << temp;


        return Output.str();




        // std::smatch matches;

        // {   // filter out uniform blocks and modify the binding qualifier 
        //     // to empty string (for OpenGL) or "{binding}" (for Vulkan)
        //     // and add the modify result to ShaderParameterCodes
            

        //     std::string temp = RawSourceCode;
        //     while (std::regex_search(temp, matches, re_uniformblock))
        //     {
        //         NILOU_LOG(Info, "Uniform block/Shader storage buffer found: "+matches[0].str());
        //         std::string prefix = matches[1];
        //         std::string buffer_type = matches[2];
        //         std::string buffer_name = matches[3];
        //         std::string body = matches[4];
        //         std::string suffix = matches[5];
                
        //         if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
        //         {      
        //             prefix = std::regex_replace(prefix, re_binding1, "");
        //             prefix = std::regex_replace(prefix, re_binding2, "");
        //             prefix = std::regex_replace(prefix, re_binding3, "(std140)");     
        //         }
        //         else 
        //         {
        //             std::regex re_binding4("\\(.*binding.*\\)");
        //             std::regex re_binding5("\\((.*)\\)");
        //             prefix = std::regex_replace(prefix, re_binding1, "{binding}");
        //             prefix = std::regex_replace(prefix, re_binding2, "{binding}");
        //             prefix = std::regex_replace(prefix, re_binding3, "(std140, {binding})");

        //             // filter out those qualifiers that are not "binding=N"
        //             if (!std::regex_search(prefix, matches, re_binding4))
        //             {
        //                 std::regex_search(prefix, matches, re_binding5);
        //                 std::string in_brackets = matches[1];
        //                 prefix = "layout(" + in_brackets + ", {binding}) uniform";
        //             }
        //         }

        //         FShaderParsedParameter ParsedParameter;
        //         if (buffer_type == "uniform")
        //             ParsedParameter.ParameterType = EShaderParameterType::SPT_UniformBuffer;
        //         else if (buffer_type == "buffer")
        //             ParsedParameter.ParameterType = EShaderParameterType::SPT_ShaderStructureBuffer;
        //         ParsedParameter.Name = buffer_name;
        //         ParsedParameter.Code = prefix + " " + buffer_name + " {" + body + "}" + suffix;
        //         ParsedResult.ParsedParameters.push_back(ParsedParameter);
        //         temp = matches.suffix();
        //     }
        // }

        // {   // filter out uniform variables 
        //     // and add the modify result to ShaderParameterCodes
            
        //     std::string temp = RawSourceCode;
        //     while (std::regex_search(temp, matches, re_uniform))
        //     {
        //         NILOU_LOG(Info, "sampler found: "+matches[0].str());
        //         std::string parameter_type = matches[1];
        //         std::string buffer_name = matches[2];
        //         FShaderParsedParameter ParsedParameter;
        //         if (GameStatics::StartsWith(parameter_type, "sampler"))
        //         {
        //             ParsedParameter.ParameterType = EShaderParameterType::SPT_Sampler;
        //             ParsedParameter.Name = buffer_name;
        //             if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
        //                 ParsedParameter.Code = "uniform " + parameter_type + " " + buffer_name + ";\n";
        //             else if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        //                 ParsedParameter.Code = "layout({binding}) uniform " + parameter_type + " " + buffer_name + ";\n";
        //             ParsedResult.ParsedParameters.push_back(ParsedParameter);
        //         }
        //         else 
        //         {
        //             NILOU_LOG(Error, 
        //                 "All uniform variables must be inside a uniform block but " 
        //                 + buffer_name + " is out of a uniform block")
        //             // Code.ParameterType = EShaderParameterType::SPT_Uniform;
        //         }
        //         temp = matches.suffix();
        //     }

        //     // like layout(...) uniform sampler2D tex; / layout(...) uniform int a;
        //     std::regex re_sampler("^[ ]*layout[ ]*\\(.*\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
        //     RawSourceCode = std::regex_replace(RawSourceCode, re_uniformblock, "");
        //     RawSourceCode = std::regex_replace(RawSourceCode, re_sampler, "");
        //     RawSourceCode = std::regex_replace(RawSourceCode, re_uniform, "");
        // }
        // return RawSourceCode;
    }
}