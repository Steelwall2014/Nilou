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
#include "Common/ContentManager.h"
#include "GameStatics.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "ShaderMap.h"
#include "ShaderType.h"
// #include "Shadinclude.h"
#include "VertexFactory.h"
#include "Common/Log.h"

#ifdef NILOU_DEBUG
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
                    std::string SourceCode = nilou::GetAssetLoader()->SyncOpenAndReadText(absolute_path.generic_string().c_str());
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
        std::set<FShaderParameterInfo> &OutShaderParameters, 
        std::array<FShaderParserResult*, N> ParsedResults, 
        const FShaderCompilerEnvironment &Environment,
        FDynamicRHI *DynamicRHI)
    {
        std::stringstream stream;
        stream << "#version 460\n";
        if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
            stream << "#define USING_OPENGL 1\n";

        for (auto &[key, value] : Environment.Definitions)
            stream << "#define " << key << " " << value << "\n";
        for (FShaderParserResult* ParsedResult : ParsedResults)
        {
            for (const FShaderParsedParameter &ParsedParameter : ParsedResult->ParsedParameters)
            {
                if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
                {
                    // for OpenGL, the binding point of images will be determined by explicit binding qualifier
                    if (ParsedParameter.ParameterType == EShaderParameterType::SPT_Image)
                    {
                        std::regex re("binding[ ]*=[ ]*([0-9])");
                        std::smatch matches;
                        if (std::regex_search(ParsedParameter.Code, matches, re))
                        {
                            OutShaderParameters.emplace(ParsedParameter.Name, stoi(matches[1].str()), ParsedParameter.ParameterType);
                        }
                    }
                    // for OpenGL, the binding point of samplers and uniform blocks will be determined when pipeline state is created
                    else 
                    {                        
                        OutShaderParameters.emplace(ParsedParameter.Name, -1, ParsedParameter.ParameterType);
                    }
                }
                // else if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
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

        // if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
        // {
        //     std::regex re("\\{binding\\}");
        //     std::string Output = stream.str();
        //     std::smatch match;
        //     while (std::regex_search(Output, match, re))
        //     {
        //         int binding_point = ParsedParameters.size();
        //         Output = std::regex_replace(Output, re, "binding="+std::to_string(binding_point), std::regex_constants::format_first_only);
        //     }
        // }

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
        FDynamicRHI *DynamicRHI, 
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<1>(
            ShaderInstance->Parameters, {&ShaderType->ParsedResult}, Environment, DynamicRHI);
        #ifdef NILOU_DEBUG
            ShaderInstance->DebugCode = code;
            Write(ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+".glsl", code);
        #endif
        switch (ShaderType->ShaderFrequency) 
        {
            case EShaderFrequency::SF_Vertex:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreateVertexShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;
                break;
            case EShaderFrequency::SF_Pixel:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreatePixelShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;
                break;
            case EShaderFrequency::SF_Compute:
                ShaderInstance->ShaderRHI = DynamicRHI->RHICreateComputeShader(code.c_str());
                ShaderInstance->PipelineStage = EPipelineStage::PS_Compute;
                break;
        }

        // AddGlobalShaderInstance(ShaderInstance, ShaderParameter);
        FContentManager::GetContentManager().AddGlobalShader(ShaderParameter, ShaderInstance);
    }

    void FShaderCompiler::CompileVertexMaterialShader(
        FDynamicRHI *DynamicRHI,
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
            ShaderInstance->Parameters, 
            {&Material->ParsedResult, &VertexFactoryType->ParsedResult, &ShaderType->ParsedResult}, 
            Environment, DynamicRHI);
        #ifdef NILOU_DEBUG
            ShaderInstance->DebugCode = code;
            Write(
                ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+" "+
                Material->GetMaterialName()+" "+
                VertexFactoryType->Name+std::to_string(VertexFactoryParams.PermutationId)+".glsl", code);
        #endif
        ShaderInstance->ShaderRHI = DynamicRHI->RHICreateVertexShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Vertex;
        
        OutShaderMap.AddShader(ShaderInstance, VertexFactoryParams, ShaderParameter);
    }

    void FShaderCompiler::CompilePixelMaterialShader(
        FDynamicRHI *DynamicRHI, 
        FMaterial *Material, 
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FShaderPermutationParameters> &OutShaderMap)
    {
        FShaderType *ShaderType = ShaderParameter.Type;

        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>();
        std::string code = ConcateShaderCodeAndParameters<2>(
            ShaderInstance->Parameters, 
            {&Material->ParsedResult, &ShaderType->ParsedResult}, 
            Environment, DynamicRHI);
        #ifdef NILOU_DEBUG
            ShaderInstance->DebugCode = code;
            Write(
                ShaderType->Name+std::to_string(ShaderParameter.PermutationId)+" "+
                Material->GetMaterialName()+".glsl", code);
        #endif
        ShaderInstance->ShaderRHI = DynamicRHI->RHICreatePixelShader(code.c_str());
        ShaderInstance->PipelineStage = EPipelineStage::PS_Pixel;

        OutShaderMap.AddShader(ShaderInstance, ShaderParameter);
    }

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

    void FShaderCompiler::CompileGlobalShaders(FDynamicRHI *DynamicRHI)
    {
        ForEachShader(
            [DynamicRHI](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileGlobalShader(DynamicRHI, ShaderParameter);
            },
            [](FShaderType *ShaderType) 
            {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Global;
            });
    }
    
    void FShaderCompiler::CompileMaterialShader(FMaterial *Material, FDynamicRHI *DynamicRHI)
    {
        ForEachShader(
            [Material, DynamicRHI](const FShaderPermutationParameters &ShaderParameter) {   
                FShaderType *ShaderType = ShaderParameter.Type;             
                if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Vertex)
                {
                    ForEachVertexFactory([Material, &ShaderParameter, DynamicRHI](const FVertexFactoryPermutationParameters &VFParameters) {
                        CompileVertexMaterialShader(
                            DynamicRHI, Material, VFParameters, ShaderParameter, 
                            Material->ShaderMap.VertexShaderMap);
                    }, 
                    [](FVertexFactoryType *) { return true; });
                }
                else if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Pixel)
                {
                    CompilePixelMaterialShader(DynamicRHI, Material, ShaderParameter, Material->ShaderMap.PixelShaderMap);
                }
            },
            [](FShaderType *ShaderType) {
                return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Material;
            });

    }
}