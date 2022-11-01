#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <vector>

#include "DynamicRHI.h"
#include "ShaderCompiler.h"
#include "Common/AssertionMacros.h"
#include "GameStatics.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "ShaderMap.h"
#include "ShaderType.h"
// #include "Shadinclude.h"
#include "VertexFactory.h"
#include "Common/Log.h"

#ifdef _DEBUG
#include <fstream>
void Write(std::string filename, std::string code)
{
    std::ofstream out(filename);
    out << code;
}
#endif

namespace fs = std::filesystem;

namespace nilou {

    std::string ProcessCodeIncludePath(const std::string &RawSourceCode, const std::filesystem::path &FileParentDir)
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

    std::string ProcessCodeIncludeInternal(const std::string &RawSourceCode, std::set<std::string> &AlreadyIncludedPathes)
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
                    std::string SourceCode = nilou::g_pAssetLoader->SyncOpenAndReadText(absolute_path.generic_string().c_str());
                    AlreadyIncludedPathes.insert(absolute_path.generic_string());
                    fullSourceCode << ProcessCodeIncludeInternal(SourceCode, AlreadyIncludedPathes);
                }
            }
            else 
            {
                fullSourceCode << lineBuffer + '\n';
            }
        }

        return fullSourceCode.str();
    }

    std::string ProcessCodeInclude(const std::string &RawSourceCode, const std::filesystem::path &FileParentDir)
    {
        std::set<std::string> AlreadyIncludedPathes;
        std::string NewRawSourceCode = ProcessCodeIncludePath(RawSourceCode, FileParentDir);
        return ProcessCodeIncludeInternal(NewRawSourceCode, AlreadyIncludedPathes);
    }

    std::string ProcessCodeShaderParams(
        const std::string &InRawSourceCode,
        std::set<FShaderParameterCode> &ShaderParameterCodes)
    {
        std::smatch matches;
        std::string RawSourceCode = InRawSourceCode;
        // like layout(...) uniform Name {
        //      mat4 matrix
        // };
        std::regex re_uniformblock("[ ]*(layout[ ]*\\(.*\\)[ ]*(uniform|buffer)[ ]+)([a-zA-Z_]+\\w*)[\\s]*\\{([\\w\\s;\\[\\]]+)\\}([.\n]*;)[\\s]*");

        {   // filter out uniform blocks and modify the binding qualifier 
            // to empty string (for OpenGL) or "{binding}" (for Vulkan)
            // and add the modify result to ShaderParameterCodes

            std::regex re_binding1(",[ ]*binding[ ]*=[ ]*[0-9]+[ ]*");      // like layout(std140, binding = 0)
            std::regex re_binding2("[ ]*binding[ ]*=[ ]*[0-9]+,[ ]*");      // like layout(binding = 0, std140)
            std::regex re_binding3("\\([ ]*binding[ ]*=[ ]*[0-9]+[ ]*\\)"); // like layout(binding = 0)
            

            std::string temp = RawSourceCode;
            while (std::regex_search(temp, matches, re_uniformblock))
            {
                NILOU_LOG(Info, "Uniform block/Shader storage buffer found: "+matches[0].str());
                std::string prefix = matches[1];
                std::string buffer_type = matches[2];
                std::string buffer_name = matches[3];
                std::string body = matches[4];
                std::string suffix = matches[5];
                
                if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                {      
                    prefix = std::regex_replace(prefix, re_binding1, "");
                    prefix = std::regex_replace(prefix, re_binding2, "");
                    prefix = std::regex_replace(prefix, re_binding3, "(std140)");     
                }
                else 
                {
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

                FShaderParameterCode Code;
                if (buffer_type == "uniform")
                    Code.ParameterType = EShaderParameterType::SPT_UniformBuffer;
                else if (buffer_type == "buffer")
                    Code.ParameterType = EShaderParameterType::SPT_ShaderStructureBuffer;
                Code.Name = buffer_name;
                Code.Code = prefix + " " + buffer_name + " {" + body + "}" + suffix;
                ShaderParameterCodes.insert(Code);
                temp = matches.suffix();
            }
        }

        {   // filter out uniform variables 
            // and add the modify result to ShaderParameterCodes

            // like uniform sampler2D tex; / uniform int a;
            std::regex re_uniform("[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
            
            std::string temp = RawSourceCode;
            while (std::regex_search(temp, matches, re_uniform))
            {
                NILOU_LOG(Info, "sampler found: "+matches[0].str());
                std::string parameter_type = matches[1];
                std::string buffer_name = matches[2];
                FShaderParameterCode Code;
                if (GameStatics::StartsWith(parameter_type, "sampler"))
                {
                    Code.ParameterType = EShaderParameterType::SPT_Sampler;
                    Code.Name = buffer_name;
                    if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                        Code.Code = "uniform " + parameter_type + " " + buffer_name + ";\n";
                    else if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
                        Code.Code = "layout({binding}) uniform " + parameter_type + " " + buffer_name + ";\n";
                    ShaderParameterCodes.insert(Code);
                }
                else 
                {
                    NILOU_LOG(Error, 
                        "All uniform variables must be inside a uniform block but " 
                        + buffer_name + " is out of a uniform block")
                    // Code.ParameterType = EShaderParameterType::SPT_Uniform;
                }
                temp = matches.suffix();
            }

            // like layout(...) uniform sampler2D tex; / layout(...) uniform int a;
            std::regex re_sampler("^[ ]*layout[ ]*\\(.*\\)[ ]*uniform[ ]+([a-zA-Z_]+\\w*)[ ]+([a-zA-Z_]+\\w*)[ ]*;[ ]*\\n");
            RawSourceCode = std::regex_replace(RawSourceCode, re_uniformblock, "");
            RawSourceCode = std::regex_replace(RawSourceCode, re_sampler, "");
            RawSourceCode = std::regex_replace(RawSourceCode, re_uniform, "");
        }
        return RawSourceCode;

    }
}

namespace nilou {


    std::map<std::string, std::string> &GetGShaderSourceDirectoryMappings()
    {
        static std::map<std::string, std::string> GShaderSourceDirectoryMappings;
        return GShaderSourceDirectoryMappings;
    }

    void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory)
    {
        check(std::filesystem::exists(RealShaderDirectory));
        check(GetGShaderSourceDirectoryMappings().count(VirtualShaderDirectory) == 0);
        GetGShaderSourceDirectoryMappings()[VirtualShaderDirectory] = RealShaderDirectory;
    }

    std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath)
    {
        bool RealFilePathFound = false;
        std::filesystem::path RealFilePath;
        std::filesystem::path ParentVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).parent_path();
        std::filesystem::path RelativeVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).filename();

        while (!ParentVirtualDirectoryPath.empty() && ParentVirtualDirectoryPath.generic_string() != "/")
        {
            if (GetGShaderSourceDirectoryMappings().count(ParentVirtualDirectoryPath.generic_string()) != 0)
            {
                RealFilePath = 
                    std::filesystem::path(GetGShaderSourceDirectoryMappings()[ParentVirtualDirectoryPath.generic_string()]) / RelativeVirtualDirectoryPath;
                RealFilePathFound = true;
                break;
            }

            RelativeVirtualDirectoryPath = ParentVirtualDirectoryPath.filename() / RelativeVirtualDirectoryPath;
            ParentVirtualDirectoryPath = ParentVirtualDirectoryPath.parent_path();
        }
        if (!RealFilePathFound)
            std::cout << "[ERROR] Can't map virtual shader source path " << VirtualFilePath << std::endl;
        return RealFilePath.generic_string();
    }

    void AddUniformsToSStream(const std::set<FShaderParameterCode> &ParameterCodes, std::stringstream &Out)
    {
        for (auto &ParameterCode : ParameterCodes)
            Out << ParameterCode.Code << "\n";
    }

    std::stringstream &operator<<(std::stringstream &out, const std::set<FShaderParameterCode> &ParameterCodes)
    {
        for (auto &ParameterCode : ParameterCodes)
            out << ParameterCode.Code << "\n";
        return out;
    }

    template<int N>
    std::string ConcateShaderCodeAndParameters(
        std::map<std::string, FShaderParameterInfo> &ParameterMap, 
        std::array<FShaderParserResult*, N> ParsedResults, 
        const FShaderCompilerEnvironment &Environment)
    {
        std::stringstream stream;
        stream << "#version 460\n";
        if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
            stream << "#define USING_OPENGL 1\n";

        for (auto [key, value] : Environment.Definitions)
            stream << "#define " << key << " " << value << "\n";
        for (FShaderParserResult* ParsedResult : ParsedResults)
        {
            for (const FShaderParsedParameter &ParsedParameter : ParsedResult->ParsedParameters)
            {
                if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                {
                    // stream << ParsedParameter.Code << "\n";
                    // for OpenGL, the binding point won't be determined until pipeline state is created
                    ParameterMap[ParsedParameter.Name] = FShaderParameterInfo(ParsedParameter.Name, -1, ParsedParameter.ParameterType);
                }
                // else if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
                // {
                //     int binding_point = ParameterMap.size();
                //     std::regex re("\\{binding\\}");
                //     std::string Code = std::regex_replace(ParsedParameter.Code, re, "binding="+std::to_string(binding_point));
                //     // stream << Code << "\n";
                //     ParameterMap[ParsedParameter.Name] = FShaderParameterInfo(ParsedParameter.Name, binding_point, ParsedParameter.ParameterType);
                // }
            }

            stream << ParsedResult->MainCode;
        }

        if (GDynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        {
            std::regex re("\\{binding\\}");
            std::string Output = stream.str();
            std::smatch match;
            while (std::regex_search(Output, match, re))
            {
                int binding_point = ParameterMap.size();
                Output = std::regex_replace(Output, re, "binding="+std::to_string(binding_point), std::regex_constants::format_first_only);
            }
        }

        return stream.str();
    }

    // template<int N>
    // std::vector<FShaderParameterInfo> ConcateShaderParameters(std::array<const FShaderTypeBase *, N> ShaderTypes)
    // {
    //     std::vector<FShaderParameterInfo> ParameterMap;
    //     for (const FShaderTypeBase *ShaderType : ShaderTypes)
    //     {
    //         for (auto &ParameterCode : ShaderType->ShaderParameterCodes)
    //         {
    //             ParameterMap.push_back(FShaderParameterInfo(ParameterCode.Name, ParameterMap.size()));
    //         }
    //     }
    //     return ParameterMap;
    // }

    void FShaderCompiler::CompileGlobalShader(
        FDynamicRHI *RHICmdList, 
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<1>(
            ShaderInstance->ParameterMap, {&ShaderType->ParsedResult}, Environment);
        #ifdef _DEBUG
            ShaderInstance->DebugCode = code;
            Write(ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+".glsl", code);
        #endif
        switch (ShaderType->ShaderFrequency) 
        {
            case EShaderFrequency::SF_Vertex:
                ShaderInstance->Shader = RHICmdList->RHICreateVertexShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;
                break;
            case EShaderFrequency::SF_Pixel:
                ShaderInstance->Shader = RHICmdList->RHICreatePixelShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;
                break;
            case EShaderFrequency::SF_Compute:
                ShaderInstance->Shader = RHICmdList->RHICreateComputeShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Compute;
                break;
        }

        // AddGlobalShaderInstance(ShaderInstance, ShaderParameter);
        AddGlobalShaderInstance2(ShaderInstance, ShaderParameter);
    }

    void FShaderCompiler::CompileVertexMaterialShader(
        FDynamicRHI *RHICmdList,
        FMaterial *Material, 
        const FVertexFactoryPermutationParameters &VertexFactoryParams,
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap)
    {
        FVertexFactoryType *VertexFactoryType = VertexFactoryParams.Type;
        FShaderType *ShaderType = ShaderParameter.Type;

        VertexFactoryType->ReadSourceCode();

        // Material Vertex Shader
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);
        VertexFactoryType->ModifyCompilationEnvironment(VertexFactoryParams, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<3>(
            ShaderInstance->ParameterMap, 
            {&Material->ParsedResult, &VertexFactoryType->ParsedResult, &ShaderType->ParsedResult}, 
            Environment);
        #ifdef _DEBUG
            ShaderInstance->DebugCode = code;
            Write(
                ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+" "+
                Material->GetMaterialName()+" "+
                VertexFactoryType->Name+std::to_string(VertexFactoryParams.PermutationId)+".glsl", code);
        #endif
        ShaderInstance->Shader = RHICmdList->RHICreateVertexShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;
        
        // AddVertexShaderInstance(ShaderInstance, *MaterialType, *VertexFactoryType, ShaderParameter);
        // AddVertexShaderInstance2(ShaderInstance, VertexFactoryParams, Material, ShaderParameter);
        OutShaderMap.AddShader(ShaderInstance, VertexFactoryParams, ShaderParameter);
    }

    void FShaderCompiler::CompilePixelMaterialShader(
        FDynamicRHI *RHICmdList, 
        FMaterial *Material, 
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FShaderPermutationParameters> &OutShaderMap)
    {
        FShaderType *ShaderType = ShaderParameter.Type;

        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<2>(
            ShaderInstance->ParameterMap, 
            {&Material->ParsedResult, &ShaderType->ParsedResult}, 
            Environment);
        #ifdef _DEBUG
            ShaderInstance->DebugCode = code;
            Write(
                ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+" "+
                Material->GetMaterialName()+".glsl", code);
        #endif
        ShaderInstance->Shader = RHICmdList->RHICreatePixelShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;

        // AddPixelShaderInstance(ShaderInstance, *MaterialType, ShaderParameter);
        // AddPixelShaderInstance2(ShaderInstance, Material, ShaderParameter);
        OutShaderMap.AddShader(ShaderInstance, ShaderParameter);
    }

    // void FShaderCompiler::IterateOnMaterials(
    //     FDynamicRHI *RHICmdList, 
    //     const FShaderPermutationParameters &ShaderParameter)
    // {
    //     std::vector<FMaterialType *> &MaterialTypes = GetAllMaterialTypes();
    //     FShaderType *ShaderType = ShaderParameter.Type;

    //     for (FMaterialType *MaterialType : MaterialTypes)
    //     {
    //         if (MaterialType->Name == "FMaterial")
    //             continue;
    //         MaterialType->ReadSourceCode();
    //         for (int32 MaterialPermutationId = 0; MaterialPermutationId < MaterialType->PermutationCount; MaterialPermutationId++)
    //         {
    //             FMaterialPermutationParameters MaterialParameters(MaterialType, MaterialPermutationId);
    //             if (!MaterialType->ShouldCompilePermutation(MaterialParameters))
    //                 continue;
    //             if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Vertex)
    //             {
    //                 // Vertex Material Shader
    //                 IterateOnVertexFactories(RHICmdList, ShaderParameter, MaterialParameters);
    //             }
    //             else if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Pixel)
    //             {
    //                 // Pixel Material Shader
    //                 CompilePixelMaterialShader(RHICmdList, MaterialParameters, ShaderParameter);
    //             }
    //         }
    //     }
    // }

    // void FShaderCompiler::IterateOnVertexFactories(
    //     FDynamicRHI *RHICmdList, 
    //     FMaterial *Material,
    //     const FShaderPermutationParameters &ShaderParameter)
    // {
    //     std::vector<FVertexFactoryType *> &VertexFactoryTypes = GetAllVertexFactoryTypes();

    //     for (FVertexFactoryType *VertexFactoryType : VertexFactoryTypes)
    //     {
    //         if (VertexFactoryType->Name == "FVertexFactory")
    //             continue;
    //         VertexFactoryType->ReadSourceCode();
    //         for (int32 VFPermutationId = 0; VFPermutationId < VertexFactoryType->PermutationCount; VFPermutationId++)
    //         {
    //             FVertexFactoryPermutationParameters VFParameters(VertexFactoryType, VFPermutationId);
    //             if (!VertexFactoryType->ShouldCompilePermutation(VFParameters))
    //                 continue;
    //             CompileVertexMaterialShader(GDynamicRHI, Material, VFParameters, ShaderParameter);
    //         }
    //     }
    // }

    template<typename Func, typename Filter>
    void ForEachShader(Func f, Filter filter)
    {
        std::vector<FShaderType *> &ShaderTypes = GetAllShaderTypes();
        for (FShaderType *ShaderType : ShaderTypes)
        {
            if (ShaderType->ShaderFrequency == EShaderFrequency::SF_None)
                continue;
            if (!filter(ShaderType))
                continue;
            ShaderType->ReadSourceCode();
            for (int32 PermutationId = 0; PermutationId < ShaderType->PermutationCount; PermutationId++)
            {
                FShaderPermutationParameters ShaderParameter(ShaderType, PermutationId);
                if (!ShaderType->ShouldCompilePermutation(ShaderParameter))
                    continue;
                f(ShaderParameter);
            }
        }

    }

    template<typename Func, typename Filter>
    void ForEachVertexFactory(Func f, Filter filter)
    {
        std::vector<FVertexFactoryType *> &VertexFactoryTypes = GetAllVertexFactoryTypes();
        for (FVertexFactoryType *VertexFactoryType : VertexFactoryTypes)
        {
            if (VertexFactoryType->Name == "FVertexFactory")
                continue;
            if (!filter(VertexFactoryType))
                continue;
            VertexFactoryType->ReadSourceCode();
            for (int32 VFPermutationId = 0; VFPermutationId < VertexFactoryType->PermutationCount; VFPermutationId++)
            {
                FVertexFactoryPermutationParameters VFParameters(VertexFactoryType, VFPermutationId);
                if (!VertexFactoryType->ShouldCompilePermutation(VFParameters))
                    continue;
                f(VFParameters);
            }
        }

    }

    void FShaderCompiler::CompileGlobalShaders()
    {
        ForEachShader(
            [](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileGlobalShader(GDynamicRHI, ShaderParameter);
            },
            [](FShaderType *ShaderType) 
            {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Global;
            });
    }
    
    void FShaderCompiler::CompileMaterialShader(FMaterial *Material)
    {
        ForEachShader(
            [Material](const FShaderPermutationParameters &ShaderParameter) {   
                FShaderType *ShaderType = ShaderParameter.Type;             
                if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Vertex)
                {
                    ForEachVertexFactory([Material, &ShaderParameter](const FVertexFactoryPermutationParameters &VFParameters) {
                        CompileVertexMaterialShader(
                            GDynamicRHI, Material, VFParameters, ShaderParameter, 
                            Material->ShaderMap.VertexShaderMap);
                    }, 
                    [](FVertexFactoryType *) { return true; });
                }
                else if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Pixel)
                {
                    CompilePixelMaterialShader(GDynamicRHI, Material, ShaderParameter, Material->ShaderMap.PixelShaderMap);
                }
            },
            [](FShaderType *ShaderType) {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Material;
            });

    }
}