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
#include "Common/Crc.h"

#ifdef NILOU_DEBUG
#include <fstream>
void Write(std::string filename, std::string code)
{
    std::ofstream out(filename);
    out << code;
}
#endif

namespace nilou {


    // std::map<std::string, std::string> &GetGShaderSourceDirectoryMappings()
    // {
    //     static std::map<std::string, std::string> GShaderSourceDirectoryMappings;
    //     return GShaderSourceDirectoryMappings;
    // }

    // void AddShaderSourceDirectoryMapping(const std::string& VirtualShaderDirectory, const std::string& RealShaderDirectory)
    // {
    //     Ncheck(std::filesystem::exists(RealShaderDirectory));
    //     Ncheck(GetGShaderSourceDirectoryMappings().count(VirtualShaderDirectory) == 0);
    //     GetGShaderSourceDirectoryMappings()[VirtualShaderDirectory] = RealShaderDirectory;
    // }

    // std::string GetShaderAbsolutePathFromVirtualPath(const std::string &VirtualFilePath)
    // {
    //     bool RealFilePathFound = false;
    //     std::filesystem::path RealFilePath;
    //     std::filesystem::path ParentVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).parent_path();
    //     std::filesystem::path RelativeVirtualDirectoryPath = std::filesystem::path(VirtualFilePath).filename();

    //     while (!ParentVirtualDirectoryPath.empty() && ParentVirtualDirectoryPath.generic_string() != "/")
    //     {
    //         if (GetGShaderSourceDirectoryMappings().count(ParentVirtualDirectoryPath.generic_string()) != 0)
    //         {
    //             RealFilePath = 
    //                 std::filesystem::path(GetGShaderSourceDirectoryMappings()[ParentVirtualDirectoryPath.generic_string()]) / RelativeVirtualDirectoryPath;
    //             RealFilePathFound = true;
    //             break;
    //         }

    //         RelativeVirtualDirectoryPath = ParentVirtualDirectoryPath.filename() / RelativeVirtualDirectoryPath;
    //         ParentVirtualDirectoryPath = ParentVirtualDirectoryPath.parent_path();
    //     }
    //     if (!RealFilePathFound)
    //         std::cout << "[ERROR] Can't map virtual shader source path " << VirtualFilePath << std::endl;
    //     return RealFilePath.generic_string();
    // }

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

    std::string ConcateShaderCodeAndParameters(
        /*std::set<FShaderParameterInfo> &OutShaderParameters, */
        std::vector<const std::string*> PreprocessResults, 
        const FShaderCompilerEnvironment &Environment)
    {
        FDynamicRHI* DynamicRHI = FDynamicRHI::GetDynamicRHI();
        std::stringstream stream;
        stream << "#version 460\n";
        stream << "#define RHI_OPENGL (0)\n";
        stream << "#define RHI_VULKAN (1)\n";
        if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::OpenGL)
            stream << "#define RHI_API RHI_OPENGL\n";
        else if (DynamicRHI->GetCurrentGraphicsAPI() == EGraphicsAPI::Vulkan)
            stream << "#define RHI_API RHI_VULKAN\n";

        for (auto &[key, value] : Environment.Definitions)
            stream << "#define " << key << " " << value << "\n";
        for (const std::string* Code : PreprocessResults)
        {
            stream << *Code;
        }

        return stream.str();
    }

    static void AddLayoutToShaderType(FShaderTypeBase* ShaderType, int32 PermutationId, int32 SetIndex, const shader_reflection::DescriptorSetLayouts& Layouts)
    {
        if (!Layouts.contains(SetIndex))
        {
            return;
        }
        std::map<std::string, RHIDescriptorSetLayoutBinding> NameToBinding;
        std::vector<RHIDescriptorSetLayoutBinding> BindingsRHI;
        for (auto& [BindingIndex, Binding] : Layouts[SetIndex])
        {
            RHIDescriptorSetLayoutBinding BindingRHI;
            BindingRHI.BindingIndex = BindingIndex;
            BindingRHI.DescriptorType = Binding.DescriptorType;
            BindingRHI.DescriptorCount = 1; // For now, only support 1
            BindingsRHI.push_back(BindingRHI);
            NameToBinding[Binding.Name] = BindingRHI;
        }
        RHIDescriptorSetLayout* LayoutRHI = RHICreateDescriptorSetLayout(BindingsRHI);
        ShaderType->DescriptorSetLayouts[PermutationId][SetIndex] = FNamedDescriptorSetLayout(LayoutRHI, NameToBinding);
    }

    void FShaderCompiler::CompileGlobalShader(
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        std::string code = ConcateShaderCodeAndParameters(
            {&ShaderType->PreprocessedCode}, Environment);

        EShaderStage ShaderStage;
        switch (ShaderType->ShaderFrequency) 
        {
        case EShaderFrequency::SF_Vertex:
            ShaderStage = EShaderStage::Vertex;
            break;
        case EShaderFrequency::SF_Pixel:
            ShaderStage = EShaderStage::Pixel;
            break;
        case EShaderFrequency::SF_Compute:
            ShaderStage = EShaderStage::Compute;
            break;
        default:
            Ncheck(0);
        }
        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>(
            ShaderType->Name, code, ShaderStage, ShaderType->ShaderMetaType);
        ShaderInstance->InitRHI();
        for (auto& [SetIndex, Layout] : ShaderInstance->DescriptorSetLayouts)
            AddLayoutToShaderType(ShaderType, ShaderParameter.PermutationId, SetIndex, ShaderInstance->DescriptorSetLayouts);
        AddGlobalShader(ShaderParameter, ShaderInstance);
    }

    void FShaderCompiler::CompileVertexMaterialShader(
        const std::string &MaterialPreprocessedResult,
        const FVertexFactoryPermutationParameters &VertexFactoryParams,
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap)
    {
        FVertexFactoryType *VertexFactoryType = VertexFactoryParams.Type;
        FShaderType *ShaderType = ShaderParameter.Type;

        VertexFactoryType->UpdateCode();

        // Material Vertex Shader
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);
        VertexFactoryType->ModifyCompilationEnvironment(VertexFactoryParams, Environment);

        std::string code = ConcateShaderCodeAndParameters(
            {&MaterialPreprocessedResult, &VertexFactoryType->PreprocessedCode, &ShaderType->PreprocessedCode}, 
            Environment);
        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>(
            ShaderType->Name, code, EShaderStage::Vertex, ShaderType->ShaderMetaType);
        ShaderInstance->InitRHI();
        AddLayoutToShaderType(ShaderType, ShaderParameter.PermutationId, VERTEX_SHADER_SET_INDEX, ShaderInstance->DescriptorSetLayouts);
        AddLayoutToShaderType(VertexFactoryType, VertexFactoryParams.PermutationId, VERTEX_FACTORY_SET_INDEX, ShaderInstance->DescriptorSetLayouts);
        OutShaderMap.AddShader(ShaderInstance, VertexFactoryParams, ShaderParameter);
    }

    void FShaderCompiler::CompilePixelMaterialShader(
        const std::string& MaterialParsedResult,
        const FShaderPermutationParameters &ShaderParameter,
        TShaderMap<FShaderPermutationParameters> &OutShaderMap)
    {
        FShaderType *ShaderType = ShaderParameter.Type;

        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        std::string code = ConcateShaderCodeAndParameters(
            {&MaterialParsedResult, &ShaderType->PreprocessedCode}, 
            Environment);
        FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>(
            ShaderType->Name, code, EShaderStage::Pixel, ShaderType->ShaderMetaType);
        ShaderInstance->InitRHI();
        AddLayoutToShaderType(ShaderType, ShaderParameter.PermutationId, PIXEL_SHADER_SET_INDEX, ShaderInstance->DescriptorSetLayouts);
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
            ShaderType->UpdateCode();
            ShaderType->DescriptorSetLayouts.clear();
            ShaderType->DescriptorSetLayouts.resize(ShaderType->PermutationCount);
            for (int32 PermutationId = 0; PermutationId < ShaderType->PermutationCount; PermutationId++)
            {
                FShaderPermutationParameters ShaderParameter(ShaderType, PermutationId);
                if (!ShaderType->ShouldCompilePermutation(ShaderParameter))
                    continue;
                f(ShaderParameter);
            }
        }

    }

    template<typename Func>
    void ForEachGlobalShader(Func f)
    {
        ForEachShader(
            f,
            [](FShaderType *ShaderType) { return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Global; });
    }

    template<typename Func>
    void ForEachMaterialShader(Func f)
    {
        ForEachShader(
            f,
            [](FShaderType *ShaderType) { return ShaderType->ShaderMetaType == EShaderMetaType::SMT_Material; });
    }

    void FShaderCompiler::CompileGlobalShaders()
    {
        ForEachGlobalShader(
            [](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileGlobalShader(ShaderParameter);
            });
    }
    
    void FShaderCompiler::CompileMaterialShader(FMaterialShaderMap* ShaderMap,
        const std::string &MaterialParsedResult)
    {
        ForEachMaterialShader(
            [ShaderMap, &MaterialParsedResult](const FShaderPermutationParameters &ShaderParameter) {   
                FShaderType *ShaderType = ShaderParameter.Type;             
                if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Vertex)
                {
                    // Iterate over all vertex factory types
                    std::vector<FVertexFactoryType *> &VertexFactoryTypes = GetAllVertexFactoryTypes();
                    for (FVertexFactoryType *VertexFactoryType : VertexFactoryTypes)
                    {
                        if (VertexFactoryType->Name == "FVertexFactory")    // It's the base class so skip it
                            continue;
                        VertexFactoryType->UpdateCode();
                        VertexFactoryType->DescriptorSetLayouts.clear();
                        VertexFactoryType->DescriptorSetLayouts.resize(VertexFactoryType->PermutationCount);
                        for (int32 VFPermutationId = 0; VFPermutationId < VertexFactoryType->PermutationCount; VFPermutationId++)
                        {
                            FVertexFactoryPermutationParameters VFParameters(VertexFactoryType, VFPermutationId);
                            if (!VertexFactoryType->ShouldCompilePermutation(VFParameters)) // Shouldn't compile this permutation, skip it
                                continue;
                            CompileVertexMaterialShader(
                                MaterialParsedResult, VFParameters, ShaderParameter, 
                                ShaderMap->VertexShaderMap);
                        }
                    }
                }
                else if (ShaderType->ShaderFrequency == EShaderFrequency::SF_Pixel)
                {
                    CompilePixelMaterialShader(
                        MaterialParsedResult, ShaderParameter, 
                        ShaderMap->PixelShaderMap);
                }
            });

    }
}