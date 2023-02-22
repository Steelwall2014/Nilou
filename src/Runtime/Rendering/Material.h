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

namespace nilou {

    enum class EShadingModel
    {
        SM_Unlit,
        SM_DefaultLit,
        SM_Subsurface
    };

    class FMaterial
    {
        friend class FShaderCompiler;
        friend class FMaterialRenderProxy;
        friend class UMaterial;
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

        void SetParameterValue(const std::string &Name, FTexture *Texture)
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

        std::map<std::string, FTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        bool bShaderCompiled = false;

    };

    class FMaterialRenderProxy
    {
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

        void FillShaderBindings(FElementShaderBindings &OutBindings)
        { 
            for (auto &[Name, Texture] : Textures)
                OutBindings.SetElementShaderBinding(Name, Texture->GetSamplerRHI());
            for (auto &[Name, UniformBuffer] : UniformBuffers)
                OutBindings.SetElementShaderBinding(Name, UniformBuffer);
        }

        std::string Name;

        FMaterialShaderMap ShaderMap;

        std::map<std::string, FTexture *> Textures;

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

        }

        UMaterial(const std::string &InName)
            : Name(InName)
            , MaterialResource(std::make_unique<FMaterial>())
        {
            MaterialResource->Name = Name;
        }

        static UMaterial *GetDefaultMaterial();

        std::string Name;

        std::string Code;

        std::filesystem::path Path;

        void UpdateCode(const std::string &InCode, bool bRecompile=true);

        void SetParameterValue(const std::string &Name, UTexture *Texture)
        {
            Textures[Name] = Texture;
            MaterialResource->SetParameterValue(Name, Texture->GetResource());
        }

        template<typename T>
        void SetParameterValue(const std::string &Name, TUUniformBuffer<T> *UniformBuffer)
        {
            UniformBuffers[Name] = UniformBuffer;
            MaterialResource->SetParameterValue(Name, UniformBuffer->GetResource());
        }

        virtual void Serialize(nlohmann::json &json) override;

        virtual void Deserialize(nlohmann::json &json) override;

        std::shared_ptr<UMaterialInstance> CreateMaterialInstance();

        FMaterial *GetResource() { return MaterialResource.get(); }

        std::map<std::string, UTexture *> GetTextureBindings() const { return Textures; }

        std::map<std::string, UUniformBuffer *> GetUniformBufferBindings() const { return UniformBuffers; }

    protected:

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, UUniformBuffer *> UniformBuffers;

        std::unique_ptr<FMaterial> MaterialResource;
        
    };

    UCLASS()
    class UMaterialInstance : public UMaterial
    {
        GENERATE_CLASS_INFO()
    public:

        virtual void Serialize(nlohmann::json &json) override;

    };

}