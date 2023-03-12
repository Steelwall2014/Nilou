#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "UniformBuffer.h"
#include "MeshPassProcessor.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "ShaderType.h"
#include "ShaderMap.h"
#include "Templates/ObjectMacros.h"
#include "Texture.h"
#include "VertexFactory.h"
#include "ShaderParameter.h"
#include "Common/ContentManager.h"

namespace nilou {

    enum class EShadingModel : uint32
    {
        SM_Unlit = 0,
        SM_DefaultLit,
        SM_OceanSubsurface,
        SM_SkyAtmosphere,

        SM_ShadingModelNum
    };

    class FMaterial
    {
        friend class FShaderCompiler;
        friend class FMaterialRenderProxy;
        friend class UMaterial;
        friend class UMaterialInstance;
    public:

        FMaterial()
        { 
        }

        FMaterial(
            const FRasterizerStateInitializer &InRasterizerState, 
            const FDepthStencilStateInitializer &InDepthStencilState,
            const FBlendStateInitializer &InBlendState) 
            : RasterizerState(InRasterizerState)
            , DepthStencilState(InDepthStencilState)
            , BlendState(InBlendState)
        { 
        }

        std::string Name;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        std::shared_ptr<FMaterialRenderProxy> CreateRenderProxy()
        {
            return std::make_shared<FMaterialRenderProxy>(this);
        }

        void UpdateMaterialCode(const std::string &InCode, bool bRecompile=true);

        // bool UseWorldOffset() { return bUseWorldOffset; }

        void SetParameterValue(const std::string &Name, UTexture *Texture)
        {
            Textures[Name] = Texture;
        }

        void SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer)
        {
            UniformBuffers[Name] = UniformBuffer;
        }

    protected:

        // bool bUseWorldOffset = false;

        FMaterialShaderMap ShaderMap;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        bool bShaderCompiled = false;

    };

    class FMaterialRenderProxy
    {
        friend class UMaterialInstance;
    public:
        FMaterialRenderProxy(FMaterial *InMaterial)
            : Name(InMaterial->Name)
            , ShaderMap(InMaterial->ShaderMap)
            , Textures(InMaterial->Textures)
            , UniformBuffers(InMaterial->UniformBuffers)
            , StencilRefValue(InMaterial->StencilRefValue)
            , RasterizerState(InMaterial->RasterizerState)
            , DepthStencilState(InMaterial->DepthStencilState)
            , BlendState(InMaterial->BlendState)
        {

        }

        FShaderInstance *GetShader(
            const FVertexFactoryPermutationParameters VFParameter, 
            const FShaderPermutationParameters &ShaderParameter)
        {
            return ShaderMap.GetShader(VFParameter, ShaderParameter);
        }
        FShaderInstance *GetShader(const FShaderPermutationParameters &ShaderParameter)
        {
            return ShaderMap.GetShader(ShaderParameter);
        }

        void FillShaderBindings(FInputShaderBindings &OutBindings)
        { 
            for (auto &[Name, Texture] : Textures)
                OutBindings.SetElementShaderBinding(Name, Texture->GetResource()->GetSamplerRHI());
            for (auto &[Name, UniformBuffer] : UniformBuffers)
                OutBindings.SetElementShaderBinding(Name, UniformBuffer->GetRHI());
        }

        std::string Name;

        FMaterialShaderMap ShaderMap;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;
    };

    UCLASS()
    class UMaterial : public UObject
    {
        friend class UMaterialInstance;
        GENERATE_CLASS_INFO()
    public:

        UMaterial()
            : Name("")
            , MaterialResource(std::make_unique<FMaterial>())
        {
            MaterialParameters = CreateUniformBuffer<FMaterialParameters>();
            SetParameterValue("FMaterialParameters", MaterialParameters.get());

            ENQUEUE_RENDER_COMMAND(Material_RenderResources)(
                [this](FDynamicRHI* RHICmdList) {
                    MaterialParameters->Data.MaterialShadingModel = (uint32)ShadingModel;
                    MaterialParameters->InitRHI();
                });
        }

        UMaterial(const std::string &InName)
            : Name(InName)
            , MaterialResource(std::make_unique<FMaterial>())
        {
            MaterialResource->Name = Name;
            MaterialParameters = CreateUniformBuffer<FMaterialParameters>();
            SetParameterValue("FMaterialParameters", MaterialParameters.get());

            ENQUEUE_RENDER_COMMAND(Material_RenderResources)(
                [this](FDynamicRHI* RHICmdList) {
                    MaterialParameters->Data.MaterialShadingModel = (uint32)ShadingModel;
                    MaterialParameters->InitRHI();
                });
        }

        static UMaterial *GetDefaultMaterial();

        std::string Name;

        void UpdateCode(const std::string &InCode, bool bRecompile=true);

        void SetParameterValue(const std::string &Name, UTexture *Texture)
        {
            Textures[Name] = Texture->SerializationPath;
            MaterialResource->SetParameterValue(Name, Texture);
        }

        void SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer)
        {
            MaterialResource->SetParameterValue(Name, UniformBuffer);
        }

        void SetShadingModel(EShadingModel InShadingModel)
        {
            ShadingModel = InShadingModel;
            UpdateMaterialParametersRHI();
        }

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        std::shared_ptr<UMaterialInstance> CreateMaterialInstance();

        FMaterial *GetResource() { return MaterialResource.get(); }

        std::string GetMateiralCode() const { return Code; }

        std::map<std::string, std::filesystem::path> Textures;

        virtual void ReleaseRenderResources()
        {
            ENQUEUE_RENDER_COMMAND(Material_ReleaseRenderResources)(
                [this](FDynamicRHI*) {
                    MaterialResource->ShaderMap.RemoveAllShaders();
                });
        }

    protected:
        BEGIN_UNIFORM_BUFFER_STRUCT(FMaterialParameters)
            uint32 MaterialShadingModel;
        END_UNIFORM_BUFFER_STRUCT()
        TUniformBufferRef<FMaterialParameters> MaterialParameters;

        std::unique_ptr<FMaterial> MaterialResource;

        std::string Code;

        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;

        void UpdateMaterialParametersRHI();
        
    };

    UCLASS()
    class UMaterialInstance : public UMaterial
    {
        GENERATE_CLASS_INFO()
    public:
        UMaterialInstance() { }
        UMaterialInstance(UMaterial* Material);

        virtual void Serialize(FArchive &Ar) override;

    };

}