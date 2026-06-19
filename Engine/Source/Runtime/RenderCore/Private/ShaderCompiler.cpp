#include "ShaderCompiler.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "DynamicRHI.h"
#include "Logging/LogMacros.h"
#include "Misc/Crc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "ShaderMap.h"
#include "SlangUtils.h"
#include "VertexFactory.h"

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

#if !defined(NDEBUG) || defined(NILOU_DEBUG)
    static slang::CompilerOptionEntry MakeIntCompilerOption(slang::CompilerOptionName Name, int32_t Value)
    {
        slang::CompilerOptionValue ValueDesc = {};
        ValueDesc.kind = slang::CompilerOptionValueKind::Int;
        ValueDesc.intValue0 = Value;
        return { Name, ValueDesc };
    }

    static void AddSpirvDebugCompilerOptions(std::vector<slang::CompilerOptionEntry>& OutOptions)
    {
        // NonSemantic Shader DebugInfo for RenderDoc; requires direct SPIR-V emission.
        OutOptions.push_back(MakeIntCompilerOption(slang::CompilerOptionName::EmitSpirvDirectly, 1));
        OutOptions.push_back(MakeIntCompilerOption(slang::CompilerOptionName::DebugInformation, SLANG_DEBUG_INFO_LEVEL_STANDARD));
        OutOptions.push_back(MakeIntCompilerOption(slang::CompilerOptionName::Optimization, SLANG_OPTIMIZATION_LEVEL_NONE));
    }
#endif

    Slang::ComPtr<slang::ISession> createSession(const FShaderCompilerEnvironment &Environment)
    {
        initGlobalSessionIfNeeded();

        std::vector<slang::CompilerOptionEntry> SpirvCompilerOptions;
#if !defined(NDEBUG) || defined(NILOU_DEBUG)
        AddSpirvDebugCompilerOptions(SpirvCompilerOptions);
#endif

        slang::TargetDesc SpirvTarget = {};
        SpirvTarget.format = SLANG_SPIRV;
        SpirvTarget.profile = GSlangGlobalSession->findProfile("spirv_1_5");
        if (!SpirvCompilerOptions.empty())
        {
            SpirvTarget.compilerOptionEntries = SpirvCompilerOptions.data();
            SpirvTarget.compilerOptionEntryCount = static_cast<uint32_t>(SpirvCompilerOptions.size());
        }

        slang::TargetDesc GlslTarget = {};
        GlslTarget.format = SLANG_GLSL;
        GlslTarget.profile = GSlangGlobalSession->findProfile("glsl_460"); // for debug

        std::vector<slang::TargetDesc> targets = { SpirvTarget, GlslTarget };

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = targets.size();

        // Preprocessor macros
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

        // Search paths
        std::vector<const char*> searchPaths;
        for (const auto &SearchPath : Environment.SearchPaths)
        {
            searchPaths.push_back(SearchPath.c_str());
        }
        sessionDesc.searchPaths = searchPaths.data();
        sessionDesc.searchPathCount = searchPaths.size();

        Slang::ComPtr<slang::ISession> session;
        GSlangGlobalSession->createSession(sessionDesc, session.writeRef());
        return session;
    }

    slang::IModule* loadModuleFromSourceString(slang::ISession* session, const fs::path& moduleFilePath)
    {
        std::string SourceString;
        if (!FFileHelper::LoadFileToString(SourceString, moduleFilePath.generic_string()))
        {
            NILOU_LOG(Error, "Failed to load shader file: {}", moduleFilePath.generic_string());
            return nullptr;
        }
        const std::string ModuleName = moduleFilePath.stem().string();
        const std::string ModulePath = moduleFilePath.generic_string();
        slang::IModule* ShaderModule;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            ShaderModule = session->loadModuleFromSourceString(ModuleName.c_str(), ModulePath.c_str(), SourceString.c_str(), diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        return ShaderModule;
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
    
    Slang::ComPtr<slang::IEntryPoint> findEntryPointByName(slang::IModule* module, const std::string& entryPointName)
    {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        module->findEntryPointByName(entryPointName.c_str(), entryPoint.writeRef());
        if (entryPoint == nullptr)
        {
            NILOU_LOG(Error, "Failed to find entry point {} of shader module {}", entryPointName, module->getName());
        }
        return entryPoint;
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
            if (GlobalParamTypeKind == slang::TypeReflection::Kind::ParameterBlock)
            {
                slang::TypeLayoutReflection* ElementTypeLayout = GlobalParamTypeLayout->getElementTypeLayout();
                const std::string StructName = ElementTypeLayout->getName();
                FShaderParametersMetadata2* MetaData = GetShaderParametersMetadata(StructName);
                if (!MetaData)
                {
                    NILOU_LOG(Fatal, "Failed to get metadata for struct {}", StructName);
                }
                int SetIndex = GlobalParamVarLayout->getOffset(slang::ParameterCategory::SubElementRegisterSpace);
                OutDescriptorSetLayouts[SetIndex] = MetaData->DescriptorSetLayout;
            }
        }
    }

    Slang::ComPtr<slang::IBlob> getEntryPointCode(Slang::ComPtr<slang::IComponentType> linkedProgram, int32 entryPointIndex, int32 targetIndex)
    {
        Slang::ComPtr<slang::IBlob> code;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = linkedProgram->getEntryPointCode(
                entryPointIndex,
                targetIndex,
                code.writeRef(),
                diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }
        return code;
    }

    Slang::ComPtr<slang::IComponentType> compileComponents(
        slang::ISession* session,
        const std::vector<slang::IComponentType*>& componentTypes)
    {
        std::vector<slang::IComponentType*> uniqueComponents;
        uniqueComponents.reserve(componentTypes.size());
        std::unordered_set<slang::IComponentType*> seen;
        for (slang::IComponentType* c : componentTypes)
        {
            if (seen.insert(c).second)
            {
                uniqueComponents.push_back(c);
            }
        }

        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = session->createCompositeComponentType(
                uniqueComponents.data(),
                uniqueComponents.size(),
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

        return linkedProgram;
    }

    void FShaderCompiler::CompileComputeShader(
        const FShaderPermutationParameters &ShaderParameter)
    {
        FShaderType *ShaderType = ShaderParameter.Type;
        
        FShaderCompilerEnvironment Environment;
        Environment.AddSearchPath(FPaths::EngineShadersPublicDir());
        Environment.AddSearchPath(FPaths::EngineDir());
        ShaderType->ModifyCompilationEnvironment(ShaderParameter, Environment);

        Slang::ComPtr<slang::ISession> session = createSession(Environment);
        slang::IModule* ShaderModule = loadModuleFromSourceString(session, ShaderType->FileAbsolutePath);
        Slang::ComPtr<slang::IEntryPoint> entryPoint = findEntryPointByName(ShaderModule, ShaderType->EntryPointName);
        Slang::ComPtr<slang::IComponentType> linkedProgram = compileComponents(session, {
            ShaderModule,
            entryPoint.get(),
        });
        Slang::ComPtr<slang::IBlob> spirvCode = getEntryPointCode(linkedProgram, 0, 0);
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)spirvCode->getBufferPointer(), spirvCode->getBufferSize());
        Ncheck(ShaderType->ShaderFrequency == EShaderFrequency::Compute);
        RHIComputeShaderRef ShaderRHI = RHICreateComputeShader(ByteCode, ShaderType->Name);
        ShaderRHI->SlangSession = session;
        ShaderRHI->SlangComponent = linkedProgram;
        AddGlobalShader(ShaderParameter, ShaderRHI);
    }

    void FShaderCompiler::CompileMaterialGraphicsPipeline(
        const std::string& MaterialName,
        const std::string& MaterialPath,
        const FGraphicsPipelinePermutationParameters& PipelineParams,
        const FVertexFactoryPermutationParameters& VFParams,
        const FShaderCompilerEnvironment& InEnvironment,
        FMaterialPipelineMap& OutPipelineMap)
    {
        FGraphicsPipeline* Pipeline       = PipelineParams.Type;
        FShaderType*       VSType         = Pipeline->VertexShaderType;
        FShaderType*       PSType         = Pipeline->PixelShaderType;
        FVertexFactoryType* VFType        = VFParams.Type;

        const bool VSIsMaterial = VSType->ShaderMetaType == EShaderMetaType::Material;
        const bool PSIsMaterial = PSType->ShaderMetaType == EShaderMetaType::Material;

        FShaderCompilerEnvironment Environment = InEnvironment;
        Pipeline->ModifyCompilationEnvironment(PipelineParams, Environment);
        if (VSIsMaterial)
            VFType->ModifyCompilationEnvironment(VFParams, Environment);

        Slang::ComPtr<slang::ISession> session = createSession(Environment);

        slang::IModule* VSModule       = loadModuleFromSourceString(session, VSType->FileAbsolutePath);
        slang::IModule* PSModule       = loadModuleFromSourceString(session, PSType->FileAbsolutePath);
        slang::IModule* VFModule       = VSIsMaterial ? loadModuleFromSourceString(session, VFType->FileAbsolutePath) : nullptr;
        slang::IModule* MaterialModule = loadModuleFromSourceString(session, MaterialPath);

        Slang::ComPtr<slang::IEntryPoint> vsEntryPoint = findEntryPointByName(VSModule, VSType->EntryPointName);
        Slang::ComPtr<slang::IComponentType> specializedVS;
        if (VSIsMaterial)
        {
            std::string VFInputName = VFType->Name + "Input";
            specializedVS = specializeEntryPoint(vsEntryPoint, {
                { slang::SpecializationArg::Kind::Type, VFModule->getLayout()->findTypeByName(VFType->Name.c_str()) },
                { slang::SpecializationArg::Kind::Type, MaterialModule->getLayout()->findTypeByName(MaterialName.c_str()) },
                { slang::SpecializationArg::Kind::Type, VFModule->getLayout()->findTypeByName(VFInputName.c_str()) },
            });
        }

        Slang::ComPtr<slang::IEntryPoint> psEntryPoint = findEntryPointByName(PSModule, PSType->EntryPointName);
        Slang::ComPtr<slang::IComponentType> specializedPS;
        if (PSIsMaterial)
        {
            specializedPS = specializeEntryPoint(psEntryPoint, {
                { slang::SpecializationArg::Kind::Type, MaterialModule->getLayout()->findTypeByName(MaterialName.c_str()) },
            });
        }

        slang::IComponentType* vsComponent = VSIsMaterial ? specializedVS.get() : vsEntryPoint.get();
        slang::IComponentType* psComponent = PSIsMaterial ? specializedPS.get() : psEntryPoint.get();

        Slang::ComPtr<slang::IComponentType> linkedProgram = compileComponents(session, {
            vsComponent,
            psComponent,
        });

        auto spirvCode_VS = getEntryPointCode(linkedProgram, 0, 0);
        auto spirvCode_PS = getEntryPointCode(linkedProgram, 1, 0);

        NILOU_LOG(Display, "Pipeline \"{}\": Material \"{}\", VF \"{}\" p{} - VS {} bytes, PS {} bytes of SPIR-V",
            Pipeline->Name, MaterialName, VFType->Name, VFParams.PermutationId,
            spirvCode_VS->getBufferSize(), spirvCode_PS->getBufferSize());

        std::string ShaderName_VS = VSIsMaterial
            ? NFormat("{}_{}_{}_p{}_Pipeline_{}_p{}", MaterialName, VFType->Name, VSType->Name, VFParams.PermutationId, Pipeline->Name, PipelineParams.PermutationId)
            : NFormat("{}_Pipeline_{}_p{}", VSType->Name, Pipeline->Name, PipelineParams.PermutationId);
        std::string ShaderName_PS = PSIsMaterial
            ? NFormat("{}_{}_Pipeline_{}_p{}", MaterialName, PSType->Name, Pipeline->Name, PipelineParams.PermutationId)
            : NFormat("{}_Pipeline_{}_p{}", PSType->Name, Pipeline->Name, PipelineParams.PermutationId);

        TArrayView<uint8> ByteCode_VS((uint8*)spirvCode_VS->getBufferPointer(), spirvCode_VS->getBufferSize());
        TArrayView<uint8> ByteCode_PS((uint8*)spirvCode_PS->getBufferPointer(), spirvCode_PS->getBufferSize());

        RHIVertexShaderRef ShaderRHI_VS = RHICreateVertexShader(ByteCode_VS, ShaderName_VS);
        RHIPixelShaderRef ShaderRHI_PS = RHICreatePixelShader(ByteCode_PS, ShaderName_PS);

        OutPipelineMap.AddPipeline(
            PipelineParams,
            VFParams,
            RHIGraphicsPipelineShaders{
                ShaderRHI_VS,
                ShaderRHI_PS,
                std::move(session),
                std::move(linkedProgram),
            });
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

    template <typename Func>
    void ForEachGlobalShader(Func f)
    {
        ForEachShader(
            f,
            [](FShaderType* ShaderType)
            { 
                return ShaderType->ShaderMetaType == EShaderMetaType::Global && ShaderType->ShaderFrequency == EShaderFrequency::Compute;
            });
    }

    template<typename Func>
    void ForEachMaterialGraphicsPipeline(Func f)
    {
        for (FGraphicsPipeline* Pipeline : GetAllGraphicsPipelines())
        {
            if (Pipeline->VertexShaderType == nullptr || Pipeline->PixelShaderType == nullptr)
                continue;
            if (Pipeline->VertexShaderType->ShaderMetaType != EShaderMetaType::Material &&
                Pipeline->PixelShaderType->ShaderMetaType  != EShaderMetaType::Material)
                continue;

            for (int32 PipelinePerm = 0; PipelinePerm < Pipeline->PermutationCount; PipelinePerm++)
            {
                FGraphicsPipelinePermutationParameters PipelineParams(Pipeline, PipelinePerm);
                if (!Pipeline->ShouldCompilePermutation(PipelineParams))
                    continue;
                f(PipelineParams);
            }
        }
    }

    void FShaderCompiler::CompileComputeShaders()
    {
        ForEachGlobalShader(
            [](const FShaderPermutationParameters &ShaderParameter) 
            {
                CompileComputeShader(ShaderParameter);
            });
    }

    void FShaderCompiler::CompileGlobalGraphicsPipeline(
        const FGraphicsPipelinePermutationParameters& PipelineParams)
    {
        FGraphicsPipeline* Pipeline = PipelineParams.Type;
        FShaderType* VSType = Pipeline->VertexShaderType;
        FShaderType* PSType = Pipeline->PixelShaderType;

        FShaderCompilerEnvironment Environment;
        Environment.AddSearchPath(FPaths::EngineShadersPublicDir());
        Environment.AddSearchPath(FPaths::EngineDir());
        Pipeline->ModifyCompilationEnvironment(PipelineParams, Environment);

        Slang::ComPtr<slang::ISession> session = createSession(Environment);

        slang::IModule* VSModule = loadModuleFromSourceString(session, VSType->FileAbsolutePath);
        slang::IModule* PSModule = loadModuleFromSourceString(session, PSType->FileAbsolutePath);

        Slang::ComPtr<slang::IEntryPoint> vsEntryPoint = findEntryPointByName(VSModule, VSType->EntryPointName);
        Slang::ComPtr<slang::IEntryPoint> psEntryPoint = findEntryPointByName(PSModule, PSType->EntryPointName);

        Slang::ComPtr<slang::IComponentType> linkedProgram = compileComponents(session, {
            VSModule,
            vsEntryPoint,
            PSModule,
            psEntryPoint
        });

        auto vsSpirvCode = getEntryPointCode(linkedProgram, 0, 0);
        auto psSpirvCode = getEntryPointCode(linkedProgram, 1, 0);

        NILOU_LOG(Display, "Pipeline \"{}\": Compiled VS {} bytes, PS {} bytes of SPIR-V",
            Pipeline->Name, vsSpirvCode->getBufferSize(), psSpirvCode->getBufferSize());

        std::string VSName = NFormat("{}_VS_p{}", Pipeline->Name, PipelineParams.PermutationId);
        std::string PSName = NFormat("{}_PS_p{}", Pipeline->Name, PipelineParams.PermutationId);

        TArrayView<uint8> VSByteCode((uint8*)vsSpirvCode->getBufferPointer(), vsSpirvCode->getBufferSize());
        TArrayView<uint8> PSByteCode((uint8*)psSpirvCode->getBufferPointer(), psSpirvCode->getBufferSize());

        RHIVertexShaderRef VSShaderRHI = RHICreateVertexShader(VSByteCode, VSName);
        RHIPixelShaderRef PSShaderRHI = RHICreatePixelShader(PSByteCode, PSName);

        AddGlobalGraphicsPipeline(PipelineParams, VSShaderRHI, PSShaderRHI, std::move(session), std::move(linkedProgram));
    }

    void FShaderCompiler::CompileGlobalGraphicsPipelines()
    {
        std::vector<FGraphicsPipeline*>& GraphicsPipelines = GetAllGraphicsPipelines();
        for (FGraphicsPipeline* Pipeline : GraphicsPipelines)
        {
            if (Pipeline->VertexShaderType == nullptr || Pipeline->PixelShaderType == nullptr)
                continue;
            if (Pipeline->VertexShaderType->FileAbsolutePath.empty() || Pipeline->PixelShaderType->FileAbsolutePath.empty())
                continue;
            // Pipelines with any Material stage are compiled per-material by CompileMaterialShader
            if (Pipeline->VertexShaderType->ShaderMetaType == EShaderMetaType::Material ||
                Pipeline->PixelShaderType->ShaderMetaType  == EShaderMetaType::Material)
                continue;

            for (int32 PermutationId = 0; PermutationId < Pipeline->PermutationCount; PermutationId++)
            {
                FGraphicsPipelinePermutationParameters PipelineParams(Pipeline, PermutationId);
                if (!Pipeline->ShouldCompilePermutation(PipelineParams))
                    continue;

                NILOU_LOG(Display, "Compiling Graphics Pipeline \"{}\", Permutation: {}", Pipeline->Name, PermutationId);
                CompileGlobalGraphicsPipeline(PipelineParams);
            }
        }
    }
    
    void FShaderCompiler::CompileMaterialShader(
        const std::string& MaterialName,
        const std::string& MaterialPath,
        FMaterialShaderMap* ShaderMap,
        const std::string &MaterialParsedResult,
        const FShaderCompilerEnvironment &Environment)
    {
        ForEachMaterialGraphicsPipeline(
            [&](const FGraphicsPipelinePermutationParameters& PipelineParams)
            {
                FGraphicsPipeline* Pipeline = PipelineParams.Type;
                const bool VSNeedsVF = Pipeline->VertexShaderType->ShaderMetaType == EShaderMetaType::Material;

                auto compile = [&](const FVertexFactoryPermutationParameters& VFParams)
                {
                    NILOU_LOG(Display, "Material: \"{}\", Pipeline: \"{}\", VF: \"{}\"",
                        MaterialName, Pipeline->Name, VFParams.Type->Name);
                    CompileMaterialGraphicsPipeline(
                        MaterialName,
                        MaterialPath,
                        PipelineParams,
                        VFParams,
                        Environment,
                        ShaderMap->PipelineMap);
                };

                if (VSNeedsVF)
                {
                    std::vector<FVertexFactoryType*>& VertexFactoryTypes = GetAllVertexFactoryTypes();
                    for (FVertexFactoryType* VFType : VertexFactoryTypes)
                    {
                        if (VFType == &FVertexFactory::StaticType)
                            continue;
                        for (int32 VFPerm = 0; VFPerm < VFType->PermutationCount; VFPerm++)
                        {
                            FVertexFactoryPermutationParameters VFParams(VFType, VFPerm);
                            if (!VFType->ShouldCompilePermutation(VFParams))
                                continue;
                            compile(VFParams);
                        }
                    }
                }
                else
                {
                    // VS is a Global shader - no VF dimension needed; use base FVertexFactory as a sentinel key
                    compile(FVertexFactoryPermutationParameters(&FVertexFactory::StaticType, 0));
                }
            });
    }
}