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
            [Proxy, ParameterInfo, Value](RenderGraph&)
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
        MaterialRenderProxy = std::make_unique<FMaterialRenderProxy>(this);
    }

    UMaterial::~UMaterial()
    {
        ENQUEUE_RENDER_COMMAND(Material_ReleaseResources)(
            [
                ToDelete_proxy = std::move(MaterialRenderProxy)] (RenderGraph&)
            {
            });
        MaterialRenderProxy = nullptr;
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
            [InShadingModel, Proxy=GetRenderProxy()](RenderGraph& Graph) 
            {
                Proxy->ShadingModel = InShadingModel;
            });
    }

    void UMaterial::SetBlendState(FBlendStateInitializer InBlendState)
    {
        BlendState = InBlendState;
        ENQUEUE_RENDER_COMMAND(Material_SetBlendState)(
            [InBlendState, Proxy=GetRenderProxy()](RenderGraph& Graph) 
            {
                Proxy->BlendState = RHICreateBlendState(InBlendState);
            });
    }

    void UMaterial::SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState)
    {
        DepthStencilState = InDepthStencilState;
        ENQUEUE_RENDER_COMMAND(Material_SetDepthStencilState)(
            [InDepthStencilState, Proxy=GetRenderProxy()](RenderGraph& Graph) 
            {
                Proxy->DepthStencilState = RHICreateDepthStencilState(InDepthStencilState);
            });
    }

    void UMaterial::SetRasterizerState(FRasterizerStateInitializer InRasterizerState)
    {
        RasterizerState = InRasterizerState;
        ENQUEUE_RENDER_COMMAND(Material_SetRasterizerState)(
            [InRasterizerState, Proxy=GetRenderProxy()](RenderGraph& Graph) 
            {
                Proxy->RasterizerState = RHICreateRasterizerState(InRasterizerState);
            });
    }

    void UMaterial::SetStencilRefValue(uint8 InStencilRefValue)
    {
        StencilRefValue = InStencilRefValue;
        ENQUEUE_RENDER_COMMAND(Material_SetStencilRefValue)(
            [InStencilRefValue, Proxy=GetRenderProxy()](RenderGraph& Graph) 
            {
                Proxy->StencilRefValue = InStencilRefValue;
            });
    }

    UMaterialInstance* UMaterial::CreateMaterialInstance()
    {
        std::string parent_path = std::filesystem::path(GetVirtualPath()).parent_path().generic_string();
        UMaterialInstance* MaterialInstance = GetContentManager()->CreateAsset<UMaterialInstance>(Name+"_Instance", parent_path);

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

        // Copy the render proxy
        auto NewRenderProxy = MaterialInstance->MaterialRenderProxy.get();
        NewRenderProxy->ShaderMap = this->MaterialRenderProxy->ShaderMap;
        for (auto& [SetIndex, Layout] : NewRenderProxy->ShaderMap->DescriptorSetLayout)
        {
            NewRenderProxy->DescriptorSets[SetIndex] = RenderGraph::CreatePersistentDescriptorSet(Layout.get());
        }
        for (auto& [Key, Texture] : this->MaterialRenderProxy->Textures)
        {
            uint32 SetIndex = Key >> 32;
            uint32 BindingIndex = Key & 0xFFFFFFFF;
            NewRenderProxy->Textures[Key] = Texture;
            NewRenderProxy->DescriptorSets[SetIndex]->SetSampler(BindingIndex, Texture->GetTextureSRV(), Texture->GetSamplerState());
        }
        for (auto& [Key, Buffer] : this->MaterialRenderProxy->UniformBuffers)
        {
            uint32 SetIndex = Key >> 32;
            uint32 BindingIndex = Key & 0xFFFFFFFF;
            // Copy the uniform buffer instead of the refs.
            RDGBufferRef NewUniformBuffer = RenderGraph::CreatePersistentBuffer(Buffer->Name, Buffer->Desc);
            NewUniformBuffer->SetData(Buffer->Data.get(), Buffer->Desc.Size, 0);
            NewRenderProxy->UniformBuffers[Key] = NewUniformBuffer;
            NewRenderProxy->UniformBufferSRVs[Key] = RenderGraph::CreatePersistentSRV(RDGBufferSRVDesc(NewUniformBuffer.get()));
            NewRenderProxy->DescriptorSets[SetIndex]->SetUniformBuffer(BindingIndex, NewRenderProxy->UniformBufferSRVs[Key].get());
        }
        NewRenderProxy->StencilRefValue = this->MaterialRenderProxy->StencilRefValue;
        NewRenderProxy->RasterizerState = this->MaterialRenderProxy->RasterizerState;
        NewRenderProxy->DepthStencilState = this->MaterialRenderProxy->DepthStencilState;
        NewRenderProxy->BlendState = this->MaterialRenderProxy->BlendState;
        NewRenderProxy->ShadingModel = this->MaterialRenderProxy->ShadingModel;

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
        SetShaderFileVirtualPath(ShaderVirtualPath);
        // if (Ar.Node["Content"].contains("MaterialUniformBlock"))
        // {
        //     // The deserialization of MaterialUniformBlock MUST be done after the shader compilation.
        //     nlohmann::json& json = Ar.Node["Content"]["MaterialUniformBlock"];
        //     FArchive local_Ar(json, Ar);
        //     MaterialUniformBlock->Deserialize(local_Ar);
        // }
        auto BlendState = this->BlendState;
        auto RasterizerState = this->RasterizerState;
        auto DepthStencilState = this->DepthStencilState;
        auto ShaderVirtualPath = this->ShaderVirtualPath;
        auto ShadingModel = this->ShadingModel;
        auto ScalarParameterValues = this->ScalarParameterValues;
        auto VectorParameterValues = this->VectorParameterValues;
        auto TextureParameterValues = this->TextureParameterValues;
        auto Proxy = GetRenderProxy();
        ENQUEUE_RENDER_COMMAND(Material_PostDeserialize)(
            [=](RenderGraph&) 
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

    FMaterialRenderProxy::FMaterialRenderProxy(UMaterial* InMaterial)
        : Material(InMaterial)
    {
        ENQUEUE_RENDER_COMMAND(FMaterialRenderProxy_Constructor)(
            [InBlendState=InMaterial->BlendState,
             InRasterizerState=InMaterial->RasterizerState,
             InDepthStencilState=InMaterial->DepthStencilState,
             InShadingModel=InMaterial->ShadingModel,
             this](RenderGraph&) 
            {
                ShaderMap = std::make_shared<FMaterialShaderMap>();
                BlendState = RHICreateBlendState(InBlendState);
                RasterizerState = RHICreateRasterizerState(InRasterizerState);
                DepthStencilState = RHICreateDepthStencilState(InDepthStencilState);
                ShadingModel = InShadingModel;
            });
    }

    void FMaterialRenderProxy::RenderThread_UpdateShader(const std::string& ShaderCode)
    {
        ShaderMap->RemoveAllShaders();
        ShaderMap->DescriptorSetLayout.clear();
        DescriptorSets.clear();
        std::string PreprocessResult = shader_preprocess::PreprocessInclude(ShaderCode, MATERIAL_STATIC_PARENT_DIR.generic_string(), {});
        FShaderCompiler::CompileMaterialShader(ShaderMap.get(), PreprocessResult);

        // Build reflection, then we can set uniforms by name.
        // This is only used for reflection, NOT the actual shader compilation.
        std::string PixelShaderCode = 
            "#version 460\n"
            "layout (location = 0) out vec4 FragColor;\n" +  
            PreprocessResult + 
            "\nvoid main() { FragColor = vec4(0.0, 0.0, 0.0, 1.0); }";
        sr::DescriptorSetLayouts DescriptorSetLayouts = sr::ReflectShader(PixelShaderCode);

        for (auto& [SetIndex, Layout] : DescriptorSetLayouts)
        {
            Ncheckf(SetIndex != VERTEX_SHADER_SET_INDEX, "Material:{} should not have the same descriptor set index as VERTEX_SHADER_SET_INDEX ({})", Material->Name, VERTEX_SHADER_SET_INDEX);
            Ncheckf(SetIndex != PIXEL_SHADER_SET_INDEX, "Material:{} should not have the same descriptor set index as PIXEL_SHADER_SET_INDEX ({})", Material->Name, PIXEL_SHADER_SET_INDEX);
            Ncheckf(SetIndex != VERTEX_FACTORY_SET_INDEX, "Material:{} should not have the same descriptor set index as VERTEX_FACTORY_SET_INDEX ({})", Material->Name, VERTEX_FACTORY_SET_INDEX);
            std::vector<RHIDescriptorSetLayoutBinding> BindingsRHI;
            for (auto& [BindingIndex, Binding] : Layout)
            {
                RHIDescriptorSetLayoutBinding BindingRHI;
                BindingRHI.BindingIndex = BindingIndex;
                BindingRHI.DescriptorType = Binding.DescriptorType;
                BindingsRHI.push_back(BindingRHI);
            }
            RHIDescriptorSetLayout* LayoutRHI = RHICreateDescriptorSetLayout(BindingsRHI);
            ShaderMap->DescriptorSetLayout[SetIndex] = RHIDescriptorSetLayoutRef(LayoutRHI);
            DescriptorSets[SetIndex] = RenderGraph::CreatePersistentDescriptorSet(LayoutRHI);

            for (auto& [BindingIndex, Binding] : Layout)
            {
                if (Binding.DescriptorType == EDescriptorType::UniformBuffer)
                {
                    for (const sr::BlockVariable& Member : Binding.Block.Members)
                    {
                        FMaterialRenderProxy::ParameterPosition Position;
                        Position.SetIndex = SetIndex;
                        Position.BindingIndex = BindingIndex;
                        Position.Offset = Member.Offset;
                        ParameterNameToPosition[Member.Name] = Position;
                    }
                    uint32 Size = Binding.Block.Size;
                    uint64 key = UNIFORMBUFFER_KEY(SetIndex, BindingIndex);
                    std::string BufferName = fmt::format("{}_UniformBuffer_s{}_b{}", Material->Name, SetIndex, BindingIndex);
                    UniformBuffers[key] = RenderGraph::CreatePersistentBuffer(BufferName, RDGBufferDesc(Size));
                    UniformBufferSRVs[key] = RenderGraph::CreatePersistentSRV(RDGBufferSRVDesc(UniformBuffers[key].get()));
                    DescriptorSets[SetIndex]->SetUniformBuffer(BindingIndex, UniformBufferSRVs[key].get());
                }
                else if (Binding.DescriptorType == EDescriptorType::CombinedImageSampler)
                {
                    FMaterialRenderProxy::ParameterPosition Position;
                    Position.SetIndex = SetIndex;
                    Position.BindingIndex = BindingIndex;
                    ParameterNameToPosition[Binding.Name] = Position;
                }
                else 
                {
                    Ncheckf(false, "Unsupported descriptor type");
                }
            }
        }

    }

}