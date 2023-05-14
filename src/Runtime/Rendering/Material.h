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
        { 
        }

        FMaterial(const FMaterial& Other)
            : Name(Other.Name)
            , ShaderMap(Other.ShaderMap)
            , bShaderCompiled(Other.bShaderCompiled)
        { 
        }

        std::string Name;

        void UpdateMaterialCode_RenderThread(const std::string &InCode, FDynamicRHI* RHICmdList);

    protected:

        std::shared_ptr<FMaterialShaderMap> ShaderMap;

        bool bShaderCompiled = false;

    };

    class NCLASS UMaterial : public UObject
    {
        friend class UMaterialInstance;
        friend class FMaterialRenderProxy;
        GENERATE_BODY()
    public:

        UMaterial();

        UMaterial(const UMaterial& Material);

        static UMaterial *GetDefaultMaterial();

        std::string Name;

        void UpdateCode(const std::string &InCode, bool bRecompile=true);

        void SetTextureParameterValue(const std::string &Name, UTexture *Texture);

        void SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer);

        void SetScalarParameterValue(const std::string &Name, FUniformValue Uniform);

        void SetShadingModel(EShadingModel InShadingModel);

        void SetBlendState(FBlendStateInitializer InBlendState);

        void SetDepthStencilState(FDepthStencilStateInitializer InDepthStencilState);

        void SetRasterizerState(FRasterizerStateInitializer InRasterizerState);

        void SetStencilRefValue(uint8 InStencilRefValue);
        
        FMaterialRenderProxy* UMaterial::GetRenderProxy() const
        {
            return DefaultMaterialInstance;
        }

        void UpdateDataToMaterialProxy();

        void SetShaderFileVirtualPath(const std::filesystem::path& VirtualPath);

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        UMaterialInstance* CreateMaterialInstance();

        FMaterial *GetResource() { return MaterialResource; }

        std::string GetMateiralCode() const { return Code; }

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

        FMaterialRenderProxy* DefaultMaterialInstance;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        std::map<std::string, FUniformValue> Uniforms;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;

        uint8 StencilRefValue = 0;
        
    };

    class NCLASS UMaterialInstance : public UMaterial
    {
        GENERATE_BODY()
    public:
        UMaterialInstance() { }

        UMaterialInstance(UMaterial* Material)
            : UMaterial(*Material)
        {

        }

        virtual void Serialize(FArchive &Ar) override;

    };

    class FMaterialRenderProxy
    {
        friend class UMaterial;
        friend class UMaterialInstance;
    public:
        FMaterialRenderProxy() { }

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

        std::shared_ptr<FMaterialShaderMap> ShaderMap;

        std::map<std::string, UTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        std::map<std::string, FUniformValue> Uniforms;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        EShadingModel ShadingModel;

    private:

        FMaterialRenderProxy(const FMaterialRenderProxy& Other);

    };

}