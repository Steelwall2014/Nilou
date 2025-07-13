#include <fstream>
#include <shaderc/shaderc.h>
#include <spirv_reflect.h>
#include "Common/Path.h"
#include "Material.h"
#include "Common/Asset/AssetLoader.h"
#include "RHIDefinitions.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"
#include "Common/ContentManager.h"
#include "RenderingThread.h"
#include "RenderGraph.h"
#include "ShaderReflection.h"
#include "ShaderPreprocess.h"

namespace fs = std::filesystem;
namespace sr = nilou::shader_reflection;

namespace nilou {

    const fs::path MATERIAL_STATIC_PARENT_DIR = FPath::MaterialDir();
    
    const std::string MATERIAL_UNIFORM_BLOCK_NAME = "MAT_UNIFORM_BLOCK";

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
        return GetContentManager()->GetMaterialByPath("/Materials/DefaultMaterial.nasset");
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
        std::string ShaderAbsPath = FPath::VirtualPathToAbsPath(ShaderVirtualPath).generic_string();
        Code = GetAssetLoader()->SyncOpenAndReadText(ShaderAbsPath.c_str());
        UpdateCode(Code);
    }

    void UMaterial::SetScalarParameterValue(const std::string &Name, float Value)
    {
	    FMaterialParameterInfo ParameterInfo(Name);
        FScalarParameterValue* ParameterValue = GameThread_FindParameterByName(ScalarParameterValues, ParameterInfo);

        bool bForceUpdate = false;
        if(!ParameterValue)
        {
            // If there's no element for the named parameter in array yet, add one.
            ParameterValue = &ScalarParameterValues.emplace_back();
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

    void UMaterial::SetVectorParameterValue(const std::string &Name, const vec4& Value)
    {
	    FMaterialParameterInfo ParameterInfo(Name);
        FVectorParameterValue* ParameterValue = GameThread_FindParameterByName(VectorParameterValues, ParameterInfo);

        bool bForceUpdate = false;
        if(!ParameterValue)
        {
            ParameterValue = &VectorParameterValues.emplace_back();
            ParameterValue->ParameterInfo = ParameterInfo;
            bForceUpdate = true;
        }
        
	    vec4 ValueToSet = Value;
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
            ParameterValue = &TextureParameterValues.emplace_back();
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

    bool UMaterial::SetVectorParameterValueByIndex(int32 ParameterIndex, const vec4& Value)
    {
        FVectorParameterValue* ParameterValue = GameThread_FindParameterByIndex(VectorParameterValues, ParameterIndex);

        if(!ParameterValue)
        {
            return false;
        }
        
	    vec4 ValueToSet = Value;
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
        UpdateCode(Code);
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

    UMaterialInstance* UMaterial::CreateMaterialInstance()
    {
        std::string parent_path = std::filesystem::path(GetVirtualPath()).parent_path().generic_string();
        UMaterialInstance* MaterialInstance = GetContentManager()->CreateAsset<UMaterialInstance>(GetName()+"_Instance", parent_path);

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

    void UMaterial::PostSerialize(FArchive& Ar)
    {
        // nlohmann::json& json = Ar.Node["Content"]["MaterialUniformBlock"];
        // FArchive local_Ar(json, Ar);
        // MaterialUniformBlock->Serialize(local_Ar);
    }

    void UMaterial::PostDeserialize(FArchive& Ar)
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

    FMaterialRenderProxy::FMaterialRenderProxy(UMaterial* InMaterial)
        : Owner(InMaterial)
    {
    }

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
        std::string PreprocessResult = shader_preprocess::PreprocessInclude(ShaderCode, MATERIAL_STATIC_PARENT_DIR.generic_string(), {});
        NILOU_LOG(Display, "Compile the shaderMap of Material {}", Owner->ShaderVirtualPath);
        FShaderCompilerEnvironment Environment;
        Environment.SetDefine("MATERIAL_SHADINGMODEL", (int32)Owner->ShadingModel);
        FShaderCompiler::CompileMaterialShader(Owner->GetName(), ShaderMap.get(), PreprocessResult, Environment);

        // Build reflection, then we can set uniforms by name.
        // This is only used for reflection, NOT the actual shader compilation.
        std::string PixelShaderCode = 
            "#version 460\n"
            "layout (location = 0) out vec4 FragColor;\n" +  
            PreprocessResult + 
            "\nvoid main() { FragColor = vec4(0.0, 0.0, 0.0, 1.0); }";
        size_t pos = 0;
        while ((pos = PixelShaderCode.find("#define BINDING_INDEX 0", pos)) != std::string::npos)
        {
            PixelShaderCode.replace(pos, 24, "");
            pos += 1;
        }
        pos = 0;
        int binding_index = 0;
        while ((pos = PixelShaderCode.find("BINDING_INDEX", pos)) != std::string::npos)
        {
            PixelShaderCode.replace(pos, 13, std::to_string(binding_index++));
            pos += 1;
        }

        std::string ReflectMessage;
        std::unordered_map<uint32, RHIDescriptorSetLayoutRef> DescriptorSetLayouts;
        std::optional<RHIPushConstantRange> PushConstantRange;
        if (!ReflectShader(PixelShaderCode, EShaderStage::Pixel, DescriptorSetLayouts, PushConstantRange, ReflectMessage))
        {
            NILOU_LOG(Error, "Error occured during material reflection of {} : \"{}\"", Owner->GetName(), ReflectMessage);
            NILOU_LOG(Error, "\n{}", PixelShaderCode);
            Ncheck(false);
        }

        for (auto& [SetIndex, Layout] : DescriptorSetLayouts)
        {
            std::vector<RHIDescriptorSetLayoutBinding> BindingsRHI;
            for (auto& Binding : Layout->Bindings)
            {
                if (Binding.Name == MATERIAL_UNIFORM_BLOCK_NAME && Binding.DescriptorType == EDescriptorType::UniformBuffer)
                {
                    std::string BufferName = NFormat("{}_UniformBuffer_s{}_b{}", Owner->GetName(), SetIndex, Binding.BindingIndex);
                    UniformBufferRDG = RenderGraph::CreatePooledBuffer(BufferName, RDGBufferDesc(Binding.BlockSize, Binding.BlockSize, EBufferUsageFlags::UniformBuffer));
                }
            }
        }
        
    }

    FMeshDrawShaderBindings FMaterialRenderProxy::GetShaderBindings() const
    {
        FMeshDrawShaderBindings ShaderBindings;
        ShaderBindings.SetBuffer(MATERIAL_UNIFORM_BLOCK_NAME, UniformBufferRDG.GetReference());
        for (auto& [Name, Texture] : TextureParameterArray)
        {
            ShaderBindings.SetTexture(Name, Texture->GetResource()->TextureRDG->GetDefaultView());
        }
        return ShaderBindings;
    }

}