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
            : ShaderMap(new FMaterialShaderMap)
            , ShadingModel(EShadingModel::SM_DefaultLit)
        { 
        }

        FMaterial(
            const FRasterizerStateInitializer &InRasterizerState, 
            const FDepthStencilStateInitializer &InDepthStencilState,
            const FBlendStateInitializer &InBlendState,
            EShadingModel InShadingModel) 
            : RasterizerState(InRasterizerState)
            , DepthStencilState(InDepthStencilState)
            , BlendState(InBlendState)
            , ShaderMap(new FMaterialShaderMap)
            , ShadingModel(InShadingModel)
        { 
        }

        std::string Name;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        EShadingModel ShadingModel;

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

        void SetParameterValue(const std::string &Name, FUniformValue UniformValue)
        {
            Uniforms.insert({Name, UniformValue});
        }

        void ReleaseResource()
        {
            FMaterialShaderMap* ToDelete = ShaderMap;
            ENQUEUE_RENDER_COMMAND(FMaterial_ReleaseResource)(
                [ToDelete](FDynamicRHI*) 
                {
                    ToDelete->RemoveAllShaders();
                    delete ToDelete;
                });
        }

    protected:

        // bool bUseWorldOffset = false;

        FMaterialShaderMap* ShaderMap;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        std::map<std::string, FUniformValue> Uniforms;

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
            , Uniforms(InMaterial->Uniforms)
            , StencilRefValue(InMaterial->StencilRefValue)
            , RasterizerState(InMaterial->RasterizerState)
            , DepthStencilState(InMaterial->DepthStencilState)
            , BlendState(InMaterial->BlendState)
            , ShadingModel(InMaterial->ShadingModel)
        {

        }

        FShaderInstance *GetShader(
            const FVertexFactoryPermutationParameters VFParameter, 
            const FShaderPermutationParameters &ShaderParameter) const
        {
            return ShaderMap->GetShader(VFParameter, ShaderParameter);
        }
        FShaderInstance *GetShader(const FShaderPermutationParameters &ShaderParameter) const
        {
            return ShaderMap->GetShader(ShaderParameter);
        }

        void FillShaderBindings(FInputShaderBindings &OutBindings)
        { 
            for (auto &[Name, Texture] : Textures)
                OutBindings.SetElementShaderBinding(Name, Texture->GetResource()->GetSamplerRHI());
            for (auto &[Name, UniformBuffer] : UniformBuffers)
                OutBindings.SetElementShaderBinding(Name, UniformBuffer->GetRHI());
            for (auto &[Name, Uniform] : Uniforms)
                OutBindings.SetUniformShaderBinding(Name, Uniform);
        }

        std::string Name;

        FMaterialShaderMap* ShaderMap;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        std::map<std::string, FUniformValue> Uniforms;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        EShadingModel ShadingModel;
    };

    UCLASS()
    class UMaterial : public UObject
    {
        friend class UMaterialInstance;
        GENERATE_CLASS_INFO()
    public:

        UMaterial()
            : Name("")
            , MaterialResource(new FMaterial())
        {
        }

        UMaterial(const std::string &InName)
            : Name(InName)
            , MaterialResource(new FMaterial())
        {
            MaterialResource->Name = Name;
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

        void SetParameterValue(const std::string &Name, FUniformValue Uniform)
        {
            MaterialResource->SetParameterValue(Name, Uniform);
        }

        void SetShadingModel(EShadingModel InShadingModel)
        {
            ShadingModel = InShadingModel;
            MaterialResource->ShadingModel = ShadingModel;
        }

        void SetShaderFileVirtualPath(const std::filesystem::path& VirtualPath);

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        UMaterialInstance* CreateMaterialInstance();

        FMaterial *GetResource() { return MaterialResource; }

        std::string GetMateiralCode() const { return Code; }

        std::map<std::string, std::filesystem::path> Textures;

        void ReleaseResources()
        {
            if (MaterialResource)
            {
                FMaterial* ToDelete = MaterialResource;
                ENQUEUE_RENDER_COMMAND(Material_ReleaseResources)(
                    [ToDelete](FDynamicRHI*) 
                    {
                        delete ToDelete;
                    });
            }
        }

    protected:

        FMaterial* MaterialResource;

        std::string Code;

        std::filesystem::path ShaderVirtualPath;

        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;
        
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