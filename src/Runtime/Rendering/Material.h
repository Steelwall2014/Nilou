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

    class NCLASS UMaterial : public NAsset
    {
        friend class UMaterialInstance;
        friend class FMaterialRenderProxy;
        GENERATED_BODY()
    public:

        UMaterial();

        UMaterial(const UMaterial& Material);

        static UMaterial *GetDefaultMaterial();

        NPROPERTY()
        std::string Name;

        NPROPERTY()
        std::string ShaderVirtualPath;

        NPROPERTY()
        std::map<std::string, UTexture *> Textures;

        NPROPERTY()
        FRasterizerStateInitializer RasterizerState;

        NPROPERTY()
        FDepthStencilStateInitializer DepthStencilState;

        NPROPERTY()
        FBlendStateInitializer BlendState;

        NPROPERTY()
        EShadingModel ShadingModel = EShadingModel::SM_DefaultLit;

        NPROPERTY()
        uint8 StencilRefValue = 0;

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

        void SetShaderFileVirtualPath(const std::string& VirtualPath);

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

        FMaterialRenderProxy* DefaultMaterialInstance;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        std::map<std::string, FUniformValue> Uniforms;
        
    };

    class NCLASS UMaterialInstance : public UMaterial
    {
        GENERATED_BODY()
    public:
        UMaterialInstance() { }

        UMaterialInstance(UMaterial* Material)
            : UMaterial(*Material)
        {

        }

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