#include <fstream>
#include "Misc/Paths.h"
#include "NObject/Package.h"
#include "Materials/Material.h"

#include "RHIDefinitions.h"
#include "NObject/ObjectMacros.h"
#include "RenderingThread.h"
#include "RenderGraph.h"
#include "ShaderPreprocess.h"
#include "Misc/FileHelper.h"
#include "ShaderCompiler.h"
#include <slang.h>
#include "SlangUtils.h"

namespace fs = std::filesystem;

namespace nilou {
    
    const std::string MATERIAL_PARAMETER_TYPE_NAME = "FMaterialParameters";
    const std::string MATERIAL_PARAMETER_VARIABLE_NAME = "MaterialParams";

    /**
    * Updates a parameter on the material instance from the game thread.
    */
    template <typename ParameterType>
    void GameThread_UpdateMIParameter(const UMaterial* Material, const ParameterType& Parameter)
    {
        FMaterialRenderProxy* Proxy = Material->GetRenderProxy();
        const FMaterialParameterInfo& ParameterInfo = Parameter.ParameterInfo;
        typename ParameterType::ValueType Value = ParameterType::GetValue(Parameter);
        ENQUEUE_RENDER_COMMAND(SetMIParameterValue)(
            [Proxy, ParameterInfo, Value](RenderGraph& Graph)
            {
                Proxy->RenderThread_UpdateParameter(ParameterInfo, Value);
            });
    }

    UMaterial *UMaterial::GetDefaultMaterial()
    {
        static UMaterial* DefaultMaterial = LoadObject<UMaterial>("/Engine/Materials/DefaultMaterial.DefaultMaterial");
        if (!DefaultMaterial)
        {
            NPackage* Pkg = CreatePackage("/Engine/Materials/DefaultMaterial");
            DefaultMaterial = NewObject<UMaterial>(Pkg, "DefaultMaterial");
            DefaultMaterial->InitializeResources();
            DefaultMaterial->SetShaderFileVirtualPath("/Shaders/Private/Materials/DefaultMaterial_Mat.slang");
            NPackage::SavePackage(Pkg);
        }
        return DefaultMaterial;
    }

    UMaterial::UMaterial()
    {
    }

    UMaterial::~UMaterial()
    {
    }

    void UMaterial::InitializeResources()
    {
        ReleaseResources();
        MaterialRenderProxy = new FMaterialRenderProxy(this);

        ENQUEUE_RENDER_COMMAND(Material_PostDeserialize)(
            [BlendState = this->BlendState
            ,RasterizerState = this->RasterizerState
            ,DepthStencilState = this->DepthStencilState
            ,ShadingModel = this->ShadingModel
            ,ScalarParameterValues = this->ScalarParameterValues
            ,VectorParameterValues = this->VectorParameterValues
            ,TextureParameterValues = this->TextureParameterValues
            ,Proxy = MaterialRenderProxy](RenderGraph& Graph) 
            {
                Proxy->BlendState = RHICreateBlendState(BlendState);
                Proxy->RasterizerState = RHICreateRasterizerState(RasterizerState);
                Proxy->DepthStencilState = RHICreateDepthStencilState(DepthStencilState);
                Proxy->ShadingModel = ShadingModel;
                for (const FScalarParameterValue& Param : ScalarParameterValues)
                {
                    Proxy->RenderThread_UpdateParameter(Param.ParameterInfo, Param.ParameterValue);
                }
                for (const FVectorParameterValue& Param : VectorParameterValues)
                {
                    Proxy->RenderThread_UpdateParameter(Param.ParameterInfo, Param.ParameterValue);
                }
                for (const FTextureParameterValue& Param : TextureParameterValues)
                {
                    Proxy->RenderThread_UpdateParameter(Param.ParameterInfo, Param.ParameterValue);
                }
            });
    }

    void UMaterial::ReleaseResources()
    {
        if (MaterialRenderProxy)
        {
            ENQUEUE_RENDER_COMMAND(Material_ReleaseResources)(
                [ToDelete_proxy = std::move(MaterialRenderProxy)] (RenderGraph&)
                {
                    delete ToDelete_proxy;
                });
            MaterialRenderProxy = nullptr;
        }
    }    

    void UMaterial::UpdateCode(const std::string &InCode)
    {
        Ncheck(GetRenderProxy());
        Code = InCode;
        ENQUEUE_RENDER_COMMAND(UMaterial_UpdateCode)(
            [InCode, Proxy=GetRenderProxy()](RenderGraph&)
            {
                Proxy->RenderThread_UpdateShader(InCode);
            }
        );
    }

    void UMaterial::SetShaderFileVirtualPath(const std::string& VirtualPath)
    {
        ShaderVirtualPath = VirtualPath;
        if (!ShaderVirtualPath.empty())
        {
            std::string ShaderAbsPath = FPaths::EngineDir() + ShaderVirtualPath;
            if (FFileHelper::LoadFileToString(Code, ShaderAbsPath))
            {
                UpdateCode(Code);
            }
            else 
            {
                NILOU_LOG(Error, "Failed to load shader file: {}", ShaderAbsPath);
            }
        }
    }

    void UMaterial::SetScalarParameterValue(const std::string &Name, float Value)
    {
	    FMaterialParameterInfo ParameterInfo(Name);
        FScalarParameterValue* ParameterValue = GameThread_FindParameterByName(ScalarParameterValues, ParameterInfo);

        bool bForceUpdate = false;
        if(!ParameterValue)
        {
            // If there's no element for the named parameter in array yet, add one.
            ParameterValue = &ScalarParameterValues.AddDefaulted_GetRef();
            ParameterValue->ParameterInfo = ParameterInfo;
            bForceUpdate = true;
        }
        
	    float ValueToSet = Value;
        // Don't enqueue an update if it isn't needed
        if (bForceUpdate || ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            // Update the material instance data in the rendering thread.
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }
    }

    void UMaterial::SetVectorParameterValue(const std::string &Name, const FVector4& Value)
    {
	    FMaterialParameterInfo ParameterInfo(Name);
        FVectorParameterValue* ParameterValue = GameThread_FindParameterByName(VectorParameterValues, ParameterInfo);

        bool bForceUpdate = false;
        if(!ParameterValue)
        {
            ParameterValue = &VectorParameterValues.AddDefaulted_GetRef();
            ParameterValue->ParameterInfo = ParameterInfo;
            bForceUpdate = true;
        }
        
	    FVector4 ValueToSet = Value;
        if (bForceUpdate || ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }
    }

    void UMaterial::SetTextureParameterValue(const std::string &Name, UTexture* Value)
    {
	    FMaterialParameterInfo ParameterInfo(Name);
        FTextureParameterValue* ParameterValue = GameThread_FindParameterByName(TextureParameterValues, ParameterInfo);

        bool bForceUpdate = false;
        if(!ParameterValue)
        {
            ParameterValue = &TextureParameterValues.AddDefaulted_GetRef();
            ParameterValue->ParameterInfo = ParameterInfo;
            bForceUpdate = true;
        }
        
	    UTexture* ValueToSet = Value;
        if (bForceUpdate || ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }
    }

    bool UMaterial::SetScalarParameterValueByIndex(int32 ParameterIndex, float Value)
    {
        FScalarParameterValue* ParameterValue = GameThread_FindParameterByIndex(ScalarParameterValues, ParameterIndex);

        if(!ParameterValue)
        {
            return false;
        }
        
	    float ValueToSet = Value;
        if (ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }

        return true;
    }

    bool UMaterial::SetVectorParameterValueByIndex(int32 ParameterIndex, const FVector4& Value)
    {
        FVectorParameterValue* ParameterValue = GameThread_FindParameterByIndex(VectorParameterValues, ParameterIndex);

        if(!ParameterValue)
        {
            return false;
        }
        
	    FVector4 ValueToSet = Value;
        if (ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }

        return true;
    }

    bool UMaterial::SetTextureParameterValueByIndex(int32 ParameterIndex, UTexture* Value)
    {
        FTextureParameterValue* ParameterValue = GameThread_FindParameterByIndex(TextureParameterValues, ParameterIndex);

        if(!ParameterValue)
        {
            return false;
        }
        
	    UTexture* ValueToSet = Value;
        if (ParameterValue->ParameterValue != ValueToSet)
        {
            ParameterValue->ParameterValue = ValueToSet;
            GameThread_UpdateMIParameter(this, *ParameterValue);
        }

        return true;
    }

    void UMaterial::SetShadingModel(EShadingModel InShadingModel)
    {
        ShadingModel = InShadingModel;
        ENQUEUE_RENDER_COMMAND(Material_SetShadingModel)(
            [InShadingModel, Proxy=GetRenderProxy()](RenderGraph&) 
            {
                Proxy->ShadingModel = InShadingModel;
            });
    }

    void UMaterial::SetBlendState(FBlendStateInitializer InBlendState)
    {
        BlendState = InBlendState;
        ENQUEUE_RENDER_COMMAND(Material_SetBlendState)(
            [InBlendState, Proxy=GetRenderProxy()](RenderGraph&) 
            {
                Proxy->BlendState = RHICreateBlendState(InBlendState);
            });
    }

    void UMaterial::SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState)
    {
        DepthStencilState = InDepthStencilState;
        ENQUEUE_RENDER_COMMAND(Material_SetDepthStencilState)(
            [InDepthStencilState, Proxy=GetRenderProxy()](RenderGraph&) 
            {
                Proxy->DepthStencilState = RHICreateDepthStencilState(InDepthStencilState);
            });
    }

    void UMaterial::SetRasterizerState(FRasterizerStateInitializer InRasterizerState)
    {
        RasterizerState = InRasterizerState;
        ENQUEUE_RENDER_COMMAND(Material_SetRasterizerState)(
            [InRasterizerState, Proxy=GetRenderProxy()](RenderGraph&) 
            {
                Proxy->RasterizerState = RHICreateRasterizerState(InRasterizerState);
            });
    }

    void UMaterial::SetStencilRefValue(uint8 InStencilRefValue)
    {
        StencilRefValue = InStencilRefValue;
        ENQUEUE_RENDER_COMMAND(Material_SetStencilRefValue)(
            [InStencilRefValue, Proxy=GetRenderProxy()](RenderGraph&) 
            {
                Proxy->StencilRefValue = InStencilRefValue;
            });
    }

    UMaterial* GetColoredMaterial()
    {
        NPackage* ColoredMaterialPackage = CreatePackage("/Engine/Materials/ColoredMaterial");
        UMaterial* ColoredMaterial = FindObject<UMaterial>(ColoredMaterialPackage, "ColoredMaterial");
        if (!ColoredMaterial)
        {
            ColoredMaterial = NewObject<UMaterial>(ColoredMaterialPackage, "ColoredMaterial");
            ColoredMaterial->InitializeResources();
            ColoredMaterial->SetRasterizerState(FRasterizerStateInitializer(ERasterizerFillMode::FM_Solid, ERasterizerCullMode::CM_None));
            ColoredMaterial->SetShaderFileVirtualPath("/Shaders/Private/Materials/ColoredMaterial_Mat.slang");
            NPackage::SavePackage(ColoredMaterialPackage);
        }
        return ColoredMaterial;
    }

    UMaterialInstance* UMaterial::CreateMaterialInstance(NPackage* Package, const std::string& Name)
    {
        UMaterialInstance* MaterialInstance = NewObject<UMaterialInstance>(Package, Name);

        // Copy some properties
        MaterialInstance->Code = this->Code;
        MaterialInstance->ScalarParameterValues = this->ScalarParameterValues;
        MaterialInstance->VectorParameterValues = this->VectorParameterValues;
        MaterialInstance->TextureParameterValues = this->TextureParameterValues;
        MaterialInstance->ShadingModel = this->ShadingModel;
        MaterialInstance->StencilRefValue = this->StencilRefValue;
        MaterialInstance->BlendState = this->BlendState;
        MaterialInstance->DepthStencilState = this->DepthStencilState;
        MaterialInstance->RasterizerState = this->RasterizerState;
        MaterialInstance->InitializeResources();

        return MaterialInstance;
    }

    void UMaterial::Serialize(FArchive& Ar)
    {
        NObject::Serialize(Ar);
    }

    void UMaterial::PostLoad()
    {
        InitializeResources();
        SetShaderFileVirtualPath(ShaderVirtualPath);
        // if (Ar.Node["Content"].contains("MaterialUniformBlock"))
        // {
        //     // The deserialization of MaterialUniformBlock MUST be done after the shader compilation.
        //     nlohmann::json& json = Ar.Node["Content"]["MaterialUniformBlock"];
        //     FArchive local_Ar(json, Ar);
        //     MaterialUniformBlock->Deserialize(local_Ar);
        // }
    }

    static void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            NILOU_LOG(Fatal, "{}", (const char*)diagnosticsBlob->getBufferPointer());
        }
    }

    FMaterialRenderProxy::FMaterialRenderProxy(UMaterial* InMaterial)
        : Owner(InMaterial)
    {
    }

    BEGIN_SHADER_PARAMETER_STRUCT(FMaterialParameters,)
        SHADER_PARAMETER(FVector4f, baseColorFactor)
    END_SHADER_PARAMETER_STRUCT()

    void FMaterialRenderProxy::RenderThread_UpdateShader(const std::string& ShaderCode)
    {
        Ncheck(IsInRenderingThread());

        if (ShaderMap)
        {
            ShaderMap->RemoveAllShaders();
        }
        else 
        {
            ShaderMap = std::make_shared<FMaterialShaderMap>();
        }
        NILOU_LOG(Display, "Compile the shaderMap of Material {}", Owner->ShaderVirtualPath);

        const std::string SearchPath = FPaths::EngineShadersPublicDir();
        const std::string MaterialPath = FPaths::EngineDir() + Owner->ShaderVirtualPath;
        const std::string MaterialModuleName = FPaths::GetBaseFilename(MaterialPath);

        FShaderCompilerEnvironment Environment;
        Environment.AddSearchPath(SearchPath);
        FShaderCompiler::CompileMaterialShader(Owner->GetName(), MaterialPath, ShaderMap.get(), ShaderCode, Environment);

        // Create a session
        slang::IGlobalSession* GlobalSession = GetSlangGlobalSession();
        slang::SessionDesc sessionDesc = {};
        std::vector<slang::TargetDesc> targets = {
            {
                .format = SLANG_SPIRV,
                .profile = GlobalSession->findProfile("spirv_1_5")
            },
        };
        sessionDesc.targets = targets.data();
        sessionDesc.targetCount = targets.size();
        std::vector<const char*> searchPaths = { SearchPath.c_str() };
        sessionDesc.searchPathCount = searchPaths.size();
        sessionDesc.searchPaths = searchPaths.data();
        Slang::ComPtr<slang::ISession> session;
        GlobalSession->createSession(sessionDesc, session.writeRef());

        // Load the material module
        slang::IModule* ShaderModule;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            ShaderModule = session->loadModuleFromSourceString(MaterialModuleName.c_str(), MaterialPath.c_str(), ShaderCode.c_str(), diagnosticsBlob.writeRef());
            diagnoseIfNeeded(diagnosticsBlob);
        }

        // Get ProgramLayout (ShaderReflection)
        slang::ProgramLayout* ProgramLayout = ShaderModule->getLayout();
        if (ProgramLayout == nullptr)
            return;

        std::string MaterialParamBlockTypeName;
        uint32_t ParameterCount = ProgramLayout->getParameterCount();
        for (uint32_t i = 0; i < ParameterCount; i++)
        {
            slang::VariableLayoutReflection* VarLayout = ProgramLayout->getParameterByIndex(i);
            if (VarLayout == nullptr)
                continue;

            const char* ParamName = VarLayout->getName();
            if (ParamName == nullptr || std::string(ParamName) != "MaterialParameters")
                continue;

            slang::TypeLayoutReflection* TypeLayout = VarLayout->getTypeLayout();
            if (TypeLayout->getKind() != slang::TypeReflection::Kind::ParameterBlock)
                continue;

            slang::TypeLayoutReflection* ElementTypeLayout = TypeLayout->getElementTypeLayout();
            if (ElementTypeLayout != nullptr)
            {
                MaterialParamBlockTypeName = ElementTypeLayout->getType()->getName();
            }
            break;
        }
        if (MaterialParamBlockTypeName.empty())
        {
            NILOU_LOG(Error, "Failed to find material parameter block type name in shader module {}", MaterialModuleName);
            return;
        }
        MaterialParamsMetadata = GetShaderParametersMetadata(MaterialParamBlockTypeName);
        if (MaterialParamsMetadata == nullptr)
        {
            NILOU_LOG(Error, "Failed to get metadata for material parameter block type {}", MaterialParamBlockTypeName);
            return;
        }
    }

    FMeshDrawShaderBindings FMaterialRenderProxy::GetShaderBindings() const
    {
        FMeshDrawShaderBindings ShaderBindings;
        ShaderBindings.SetBuffer(MATERIAL_PARAMETER_TYPE_NAME, UniformBufferRDG.GetReference());
        for (auto& [Name, Texture] : TextureParameterArray)
        {
            ShaderBindings.SetTexture(Name, Texture->GetResource()->TextureRDG->GetDefaultView());
        }
        return ShaderBindings;
    }

}