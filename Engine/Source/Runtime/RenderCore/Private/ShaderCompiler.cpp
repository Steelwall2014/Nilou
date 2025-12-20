#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <vector>

#include "DynamicRHI.h"
#include "ShaderCompiler.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "ShaderMap.h"
// #include "Shadinclude.h"
#include "VertexFactory.h"
#include "Logging/LogMacros.h"
#include "Misc/Crc.h"
#include "SlangUtils.h"

#ifdef NILOU_DEBUG
#include <fstream>
void Write(std::string filename, std::string code)
{
    std::ofstream out(filename);
    out << code;
}
#endif

namespace nilou {

    Slang::ComPtr<slang::IGlobalSession> GSlangGlobalSession = nullptr;
    slang::IGlobalSession* GetSlangGlobalSession()
    {
        return GSlangGlobalSession.get();
    }

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
        FDynamicRHI* DynamicRHI = FDynamicRHI::Get();
        std::stringstream stream;
        stream << "#version 460\n";
        stream << "#define FOR_INTELLISENSE 0\n";
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

        std::string shaderCode = stream.str();
        size_t pos = 0;
        while ((pos = shaderCode.find("#define BINDING_INDEX 0", pos)) != std::string::npos)
        {
            shaderCode.replace(pos, 24, "");
            pos += 1;
        }

        pos = 0;
        int binding_index = 0;
        while ((pos = shaderCode.find("BINDING_INDEX", pos)) != std::string::npos)
        {
            shaderCode.replace(pos, 13, std::to_string(binding_index++));
            pos += 1;
        }
        return shaderCode;
    }

    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            NILOU_LOG(Fatal, "{}", (const char*)diagnosticsBlob->getBufferPointer());
        }
    }

    void initGlobalSessionIfNeeded()
    {
        if (GSlangGlobalSession == nullptr)
        {
            slang::createGlobalSession(GSlangGlobalSession.writeRef());
        }
    }

    Slang::ComPtr<slang::ISession> createSession(const FShaderCompilerEnvironment &Environment)
    {
        initGlobalSessionIfNeeded();
        
        slang::SessionDesc sessionDesc = {};
        std::vector<slang::TargetDesc> targets = {
            {
                .format = SLANG_SPIRV,
                .profile = GSlangGlobalSession->findProfile("spirv_1_5")
            },
            {
                .format = SLANG_GLSL,
                .profile = GSlangGlobalSession->findProfile("glsl_460")
            }
        };
        std::vector<slang::PreprocessorMacroDesc> preprocessorMacros;
        for (auto &[key, value] : Environment.Definitions)
        {
            slang::PreprocessorMacroDesc preprocessorMacro = {};
            preprocessorMacro.name = key.c_str();
            preprocessorMacro.value = value.c_str();
            preprocessorMacros.push_back(preprocessorMacro);
        }
        sessionDesc.preprocessorMacros = preprocessorMacros.data();
        sessionDesc.preprocessorMacroCount = preprocessorMacros.size();
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = targets.size();

        Slang::ComPtr<slang::ISession> session;
        GSlangGlobalSession->createSession(sessionDesc, session.writeRef());
        return session;
    }

    slang::IModule* createModule(slang::ISession* session, FShaderTypeBase* ShaderType)
    {
        slang::IModule* ShaderModule;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            ShaderModule = session->loadModuleFromSourceString(ShaderType->FileAbsolutePath.filename().generic_string().c_str(), ShaderType->FileAbsolutePath.generic_string().c_str(), ShaderType->PreprocessedCode.c_str(), diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        return ShaderModule;
    }

    slang::IModule* createModuleFromSourceString(slang::ISession* session, const std::string& ModuleName, const std::string& ModulePath, const std::string& ModuleCode)
    {
        slang::IModule* Module;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            Module = session->loadModuleFromSourceString(ModuleName.c_str(), ModulePath.c_str(), ModuleCode.c_str(), diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        return Module;
    }

    Slang::ComPtr<slang::IComponentType> specializeEntryPoint(slang::IEntryPoint* entryPoint, const std::vector<slang::SpecializationArg> &specializationArgs)
    {
        Slang::ComPtr<slang::IComponentType> specializedEntryPoint;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = entryPoint->specialize(
                specializationArgs.data(),
                specializationArgs.size(),
                specializedEntryPoint.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        return specializedEntryPoint;
    }

    EDescriptorType TranslateBindingTypeToDescriptorType(slang::BindingType BindingType)
    {
        // Clear MutableFlag bits
        slang::BindingType BaseType = (slang::BindingType)((int)BindingType & (int)slang::BindingType::BaseMask);
        
        switch (BaseType)
        {
        case slang::BindingType::ConstantBuffer:
            return EDescriptorType::UniformBuffer;
        case slang::BindingType::RawBuffer:
            return EDescriptorType::StorageBuffer;
        case slang::BindingType::TypedBuffer:
            return EDescriptorType::StorageBuffer;
        case slang::BindingType::Texture:
            return EDescriptorType::SampledImage;
        case slang::BindingType::CombinedTextureSampler:
            return EDescriptorType::CombinedImageSampler;
        case slang::BindingType::Sampler:
            return EDescriptorType::Sampler;
        case slang::BindingType::MutableTexture:
        case slang::BindingType::MutableTypedBuffer:
        case slang::BindingType::MutableRawBuffer:
            return EDescriptorType::StorageImage;
        default:
            NILOU_LOG(Warning, "Unknown binding type: {}, defaulting to UniformBuffer", (int)BaseType);
            return EDescriptorType::UniformBuffer;
        }
    }

    void ParseUniformBufferMembers(
        slang::TypeLayoutReflection* TypeLayout,
        std::vector<RHIDescriptorSetLayoutBinding::Member>& OutMembers,
        uint32 BaseOffset = 0)
    {
        if (TypeLayout == nullptr)
            return;

        slang::TypeReflection* Type = TypeLayout->getType();
        if (Type == nullptr)
            return;

        slang::TypeReflection::Kind Kind = Type->getKind();
        
        if (Kind == slang::TypeReflection::Kind::Struct)
        {
            uint32 FieldCount = TypeLayout->getFieldCount();
            for (uint32 i = 0; i < FieldCount; i++)
            {
                slang::VariableLayoutReflection* FieldLayout = TypeLayout->getFieldByIndex(i);
                if (FieldLayout == nullptr)
                    continue;

                slang::VariableReflection* FieldVar = FieldLayout->getVariable();
                if (FieldVar == nullptr)
                    continue;

                const char* FieldName = FieldVar->getName();
                if (FieldName == nullptr)
                    continue;

                uint32 FieldOffset = (uint32)FieldLayout->getOffset(slang::ParameterCategory::Uniform);
                uint32 AbsoluteOffset = BaseOffset + FieldOffset;
                
                RHIDescriptorSetLayoutBinding::Member Member;
                Member.Name = FieldName;
                Member.Offset = AbsoluteOffset;
                OutMembers.push_back(Member);

                // Recursively process nested structures - nested structure member offsets are relative to the nested structure itself
                slang::TypeLayoutReflection* FieldTypeLayout = FieldLayout->getTypeLayout();
                if (FieldTypeLayout != nullptr)
                {
                    slang::TypeReflection* FieldType = FieldTypeLayout->getType();
                    if (FieldType != nullptr && FieldType->getKind() == slang::TypeReflection::Kind::Struct)
                    {
                        // For nested structures, use absolute offset as the new base address when recursing
                        ParseUniformBufferMembers(FieldTypeLayout, OutMembers, AbsoluteOffset);
                    }
                }
            }
        }
    }

    void ParseSlangReflection(
        slang::IComponentType* LinkedProgram,
        std::unordered_map<uint32, TRefCountPtr<RHIDescriptorSetLayout>>& OutDescriptorSetLayouts)
    {
        if (LinkedProgram == nullptr)
            return;

        // Get ProgramLayout (ShaderReflection)
        slang::ProgramLayout* ProgramLayout = LinkedProgram->getLayout();
        if (ProgramLayout == nullptr)
            return;

        slang::VariableLayoutReflection* GlobalParamsVarLayout = ProgramLayout->getGlobalParamsVarLayout();
        slang::TypeLayoutReflection* GlobalParamsTypeLayout = GlobalParamsVarLayout->getTypeLayout();
        if (GlobalParamsTypeLayout->getKind() == slang::TypeReflection::Kind::Struct)
        {
            int GlobalParamsCount = GlobalParamsTypeLayout->getFieldCount();
            for (int i = 0; i < GlobalParamsCount; i++)
            {
                slang::VariableLayoutReflection* GlobalParamVarLayout = GlobalParamsTypeLayout->getFieldByIndex(i);
                if (GlobalParamVarLayout == nullptr)
                    continue;
                std::string GlobalParamName = GlobalParamVarLayout->getName();
                slang::TypeLayoutReflection* GlobalParamTypeLayout = GlobalParamVarLayout->getTypeLayout();
                if (GlobalParamTypeLayout == nullptr)
                    continue;
                std::string GlobalParamTypeName = GlobalParamTypeLayout->getName();
                slang::TypeReflection::Kind GlobalParamTypeKind = GlobalParamTypeLayout->getKind();
                int SetIndex = GlobalParamVarLayout->getOffset(slang::ParameterCategory::SubElementRegisterSpace);
                if (GlobalParamTypeKind == slang::TypeReflection::Kind::ParameterBlock) // ParameterBlock<T>, while T is a struct
                {
                    int BindingIndex = 0;
                    slang::TypeLayoutReflection* ElementTypeLayout = GlobalParamTypeLayout->getElementVarLayout()->getTypeLayout();
                    std::string ElementTypeName = ElementTypeLayout->getName();
                    if (ElementTypeLayout->getKind() == slang::TypeReflection::Kind::Struct)
                    {
                        int FieldCount = ElementTypeLayout->getFieldCount();
                        for (int i = 0; i < FieldCount; i++)
                        {
                            slang::VariableLayoutReflection* FieldLayout = ElementTypeLayout->getFieldByIndex(i);
                            if (FieldLayout == nullptr)
                                continue;
                                
                            std::string fieldName = FieldLayout->getName();
                        }

                        int bindingRangeCount = ElementTypeLayout->getBindingRangeCount();
                        for (int i = 0; i < bindingRangeCount; i++)
                        {
                            RHIDescriptorSetLayoutBinding Binding;
                            Binding.BindingIndex = BindingIndex++;
                            Binding.DescriptorType = TranslateBindingTypeToDescriptorType(ElementTypeLayout->getBindingRangeType(i));
                            Binding.DescriptorCount = ElementTypeLayout->getBindingRangeBindingCount(i);
                            OutDescriptorSetLayouts[SetIndex]->Bindings.push_back(Binding);
                        }
                    }
                }
                else 
                {
                    NILOU_LOG(Fatal, "Nilou Engine requires all global parameters to be a ParameterBlock<T>, while T is a struct");
                }
            }
        }
    }

    std::pair<Slang::ComPtr<slang::IBlob>, Slang::ComPtr<slang::IComponentType>> compileFromComponents(
        slang::ISession* session,
        const std::vector<slang::IComponentType*>& componentTypes)
    {
        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = composedProgram->link(
                linkedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }

        Slang::ComPtr<slang::IBlob> byteCode;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                0,
                0,
                byteCode.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }

        Slang::ComPtr<slang::IBlob> glslCode;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                0,
                1,
                glslCode.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        const char* glslCodeString = (const char*)glslCode->getBufferPointer();
        NILOU_LOG(Display, "{}", glslCodeString);
        slang::ProgramLayout* ProgramLayout = linkedProgram->getLayout();
        FSlangUtils::PrintProgramLayout(ProgramLayout, SLANG_SPIRV);
        return {byteCode, linkedProgram};
    }

    void FShaderCompiler::CompileGlobalShader(
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        RHIShaderRef ShaderRHI = nullptr;
        if (ShaderType->FileAbsolutePath.extension() == ".slang")
        {
            Slang::ComPtr<slang::ISession> session = createSession(Environment);
            slang::IModule* ShaderModule = createModule(session, ShaderType);
            Slang::ComPtr<slang::IEntryPoint> entryPoint;
            ShaderModule->findEntryPointByName(ShaderType->EntryPointName.c_str(), entryPoint.writeRef());
            auto [spirvCode, linkedProgram] = compileFromComponents(session, {
                ShaderModule,
                entryPoint
            });
            TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)spirvCode->getBufferPointer(), spirvCode->getBufferSize());
            switch (ShaderType->ShaderFrequency) 
            {
            case EShaderFrequency::Vertex:
                ShaderRHI = RHICreateVertexShader(ByteCode, ShaderType->Name);
                break;
            case EShaderFrequency::Pixel:
                ShaderRHI = RHICreatePixelShader(ByteCode, ShaderType->Name);
                break;
            case EShaderFrequency::Compute:
                ShaderRHI = RHICreateComputeShader(ByteCode, ShaderType->Name);
                break;
            default:
                Ncheck(0);
            }
            
            // Parse Slang reflection and populate DescriptorSetLayouts
            if (ShaderRHI != nullptr && linkedProgram != nullptr)
            {
                ParseSlangReflection(linkedProgram, ShaderRHI->DescriptorSetLayouts);
            }
        }
        else if (ShaderType->FileAbsolutePath.extension() == ".glsl")
        {
            std::string code = ConcateShaderCodeAndParameters(
                {&ShaderType->PreprocessedCode}, Environment);
            switch (ShaderType->ShaderFrequency) 
            {
            case EShaderFrequency::Vertex:
                ShaderRHI = RHICreateVertexShader(code, ShaderType->Name);
                break;
            case EShaderFrequency::Pixel:
                ShaderRHI = RHICreatePixelShader(code, ShaderType->Name);
                break;
            case EShaderFrequency::Compute:
                ShaderRHI = RHICreateComputeShader(code, ShaderType->Name);
                break;
            default:
                Ncheck(0);
            }
        }
        AddGlobalShader(ShaderParameter, ShaderRHI);
    }

    void FShaderCompiler::CompileVertexMaterialShader(
        const std::string& MaterialName,
        const std::string& MaterialPath,
        const std::string &MaterialPreprocessedResult,
        const FVertexFactoryPermutationParameters &VertexFactoryParams,
        const FShaderPermutationParameters &ShaderParameter,
        FShaderCompilerEnvironment &Environment,
        TShaderMap<FVertexFactoryPermutationParameters, FShaderPermutationParameters> &OutShaderMap)
    {
        FVertexFactoryType *VertexFactoryType = VertexFactoryParams.Type;
        FShaderType *ShaderType = ShaderParameter.Type;

        // Material Vertex Shader
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);
        VertexFactoryType->ModifyCompilationEnvironment(VertexFactoryParams, Environment);
        Environment.SetDefine("SET_INDEX", 0);

        Slang::ComPtr<slang::ISession> session = createSession(Environment);

        slang::IModule* ShaderModule = createModule(session, ShaderType);
        slang::IModule* VertexFactoryModule = createModule(session, VertexFactoryType);
        slang::IModule* MaterialModule = createModuleFromSourceString(session, MaterialName, MaterialPath, MaterialPreprocessedResult);

        std::string VertexFactoryInputName = VertexFactoryType->Name + "Input";
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        ShaderModule->findEntryPointByName(ShaderType->EntryPointName.c_str(), entryPoint.writeRef());
        Slang::ComPtr<slang::IComponentType> specializedEntryPoint = specializeEntryPoint(entryPoint, { 
            {
                slang::SpecializationArg::Kind::Type,
                VertexFactoryModule->getLayout()->findTypeByName(VertexFactoryType->Name.c_str())
            },
            {
                slang::SpecializationArg::Kind::Type,
                MaterialModule->getLayout()->findTypeByName(MaterialName.c_str())
            },
            {
                slang::SpecializationArg::Kind::Type,
                VertexFactoryModule->getLayout()->findTypeByName(VertexFactoryInputName.c_str())
            },
        });
        
        auto [spirvCode, linkedProgram] = compileFromComponents(session, {
            ShaderModule,
            MaterialModule,
            VertexFactoryModule,
            specializedEntryPoint
        });
        NILOU_LOG(Display, "Compiled {} bytes of SPIR-V", spirvCode->getBufferSize());

        std::string ShaderName = NFormat("{}_{}_p{}_{}_p{}", MaterialName, VertexFactoryType->Name, VertexFactoryParams.PermutationId, ShaderType->Name, ShaderParameter.PermutationId);
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)spirvCode->getBufferPointer(), spirvCode->getBufferSize());
        RHIShaderRef ShaderRHI = RHICreateVertexShader(ByteCode, ShaderName);

        // Parse Slang reflection and populate DescriptorSetLayouts
        if (ShaderRHI != nullptr && linkedProgram != nullptr)
        {
            ParseSlangReflection(linkedProgram, ShaderRHI->DescriptorSetLayouts);
        }

        OutShaderMap.AddShader(ShaderRHI, VertexFactoryParams, ShaderParameter);


        // std::string code = ConcateShaderCodeAndParameters(
        //     {&MaterialPreprocessedResult, &VertexFactoryType->PreprocessedCode, &ShaderType->PreprocessedCode}, 
        //     Environment);
        // FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>(
        //     ShaderName, code, EShaderStage::Vertex, ShaderType->ShaderMetaType);
        // ShaderInstance->InitRHI();
        // OutShaderMap.AddShader(ShaderInstance, VertexFactoryParams, ShaderParameter);
    }

    void FShaderCompiler::CompilePixelMaterialShader(
        const std::string& MaterialName,
        const std::string& MaterialPath,
        const std::string& MaterialPreprocessedResult,
        const FShaderPermutationParameters &ShaderParameter,
        FShaderCompilerEnvironment &Environment,
        TShaderMap<FShaderPermutationParameters> &OutShaderMap)
    {
        FShaderType *ShaderType = ShaderParameter.Type;

        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);
        Environment.SetDefine("SET_INDEX", 1);

        Slang::ComPtr<slang::ISession> session = createSession(Environment);
        slang::IModule* ShaderModule = createModule(session, ShaderType);
        slang::IModule* MaterialModule = createModuleFromSourceString(session, MaterialName, MaterialPath, MaterialPreprocessedResult);
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        ShaderModule->findEntryPointByName(ShaderType->EntryPointName.c_str(), entryPoint.writeRef());
        Slang::ComPtr<slang::IComponentType> specializedEntryPoint = specializeEntryPoint(entryPoint, {
            {
                slang::SpecializationArg::Kind::Type,
                MaterialModule->getLayout()->findTypeByName(MaterialName.c_str())
            },
        });

        std::vector<slang::IComponentType*> componentTypes =
        {
            ShaderModule,
            MaterialModule,
            specializedEntryPoint
        };

        auto [spirvCode, linkedProgram] = compileFromComponents(session, {
            ShaderModule,
            MaterialModule,
            specializedEntryPoint
        });
        NILOU_LOG(Display, "Compiled {} bytes of SPIR-V", spirvCode->getBufferSize());

        std::string ShaderName = NFormat("{}_{}_p{}", MaterialName, ShaderType->Name, ShaderParameter.PermutationId);
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)spirvCode->getBufferPointer(), spirvCode->getBufferSize());
        RHIShaderRef ShaderRHI = RHICreatePixelShader(ByteCode, ShaderName);
        
        // Parse Slang reflection and populate DescriptorSetLayouts
        if (ShaderRHI != nullptr && linkedProgram != nullptr)
        {
            ParseSlangReflection(linkedProgram, ShaderRHI->DescriptorSetLayouts);
        }
        
        OutShaderMap.AddShader(ShaderRHI, ShaderParameter);

        // std::string code = ConcateShaderCodeAndParameters(
        //     {&MaterialParsedResult, &ShaderType->PreprocessedCode}, 
        //     Environment);
        // FShaderInstanceRef ShaderInstance = std::make_shared<FShaderInstance>(
        //     ShaderName, code, EShaderStage::Pixel, ShaderType->ShaderMetaType);
        // ShaderInstance->InitRHI();
        // OutShaderMap.AddShader(ShaderInstance, ShaderParameter);
    }

    template<typename Func, typename Filter>
    void ForEachShader(Func f, Filter filter)
    {
        std::vector<FShaderType *> &ShaderTypes = GetAllShaderTypes();
        for (FShaderType *ShaderType : ShaderTypes)
        {
            if (ShaderType->ShaderFrequency == EShaderFrequency::None)
                continue;
            if (!filter(ShaderType))
                continue;
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
            [](FShaderType *ShaderType) { return ShaderType->ShaderMetaType == EShaderMetaType::Global; });
    }

    template<typename Func>
    void ForEachMaterialShader(Func f)
    {
        ForEachShader(
            f,
            [](FShaderType *ShaderType) { return ShaderType->ShaderMetaType == EShaderMetaType::Material; });
    }

    void FShaderCompiler::CompileGlobalShaders()
    {
        ForEachGlobalShader(
            [](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileGlobalShader(ShaderParameter);
            });
    }
    
    void FShaderCompiler::CompileMaterialShader(
        const std::string& MaterialName,
        const std::string& MaterialPath,
        FMaterialShaderMap* ShaderMap,
        const std::string &MaterialParsedResult,
        FShaderCompilerEnvironment &Environment)
    {
        ForEachMaterialShader(
            [&](const FShaderPermutationParameters &ShaderParameter) {   
                FShaderType *ShaderType = ShaderParameter.Type;             
                if (ShaderType->ShaderFrequency == EShaderFrequency::Vertex)
                {
                    // Iterate over all vertex factory types
                    // std::vector<FVertexFactoryType *> &VertexFactoryTypes = GetAllVertexFactoryTypes();
                    // for (FVertexFactoryType *VertexFactoryType : VertexFactoryTypes)
                    // {
                    //     if (VertexFactoryType == &FVertexFactory::StaticType)    // It's the base class so skip it
                    //         continue;
                    //     for (int32 VFPermutationId = 0; VFPermutationId < VertexFactoryType->PermutationCount; VFPermutationId++)
                    //     {
                    //         FVertexFactoryPermutationParameters VFParameters(VertexFactoryType, VFPermutationId);
                    //         if (!VertexFactoryType->ShouldCompilePermutation(VFParameters)) // Shouldn't compile this permutation, skip it
                    //             continue;
                    //         NILOU_LOG(Display, "Material: \"{}\", VertexFactory: \"{}\", VertexShader: \"{}\", Permutation: {}", MaterialName, VertexFactoryType->Name, ShaderType->Name, VFPermutationId);
                    //         CompileVertexMaterialShader(
                    //             MaterialName,
                    //             MaterialPath,
                    //             MaterialParsedResult, VFParameters, ShaderParameter, 
                    //             Environment,
                    //             ShaderMap->VertexShaderMap);
                    //     }
                    // }
                }
                else if (ShaderType->ShaderFrequency == EShaderFrequency::Pixel)
                {
                    NILOU_LOG(Display, "Material: \"{}\", PixelShader: \"{}\", Permutation: {}", MaterialName, ShaderType->Name, ShaderParameter.PermutationId);
                    CompilePixelMaterialShader(
                        MaterialName,
                        MaterialPath,
                        MaterialParsedResult, ShaderParameter, 
                        Environment,
                        ShaderMap->PixelShaderMap);
                }
            });

    }
}