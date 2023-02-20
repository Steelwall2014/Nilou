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
    public:

        static FMaterial *GetDefaultMaterial();

        FMaterial(const std::string &InMaterialName)
            : MaterialName(InMaterialName)
        { 
        }

        FMaterial(
            const std::string &InMaterialName,
            const FRasterizerStateInitializer &InRasterizerState, 
            const FDepthStencilStateInitializer &InDepthStencilState,
            const FBlendStateInitializer &InBlendState) 
            : MaterialName(InMaterialName)
            , RasterizerState(InRasterizerState)
            , DepthStencilState(InDepthStencilState)
            , BlendState(InBlendState)
        { 
        }

        ~FMaterial();

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        void ReleaseRenderResources();

        std::shared_ptr<FMaterialRenderProxy> CreateRenderProxy()
        {
            return std::make_shared<FMaterialRenderProxy>(this);
        }

        void SetMaterialName(const std::string Name)
        {
            MaterialName = Name;
        }

        std::string GetMaterialName()
        {
            return MaterialName;
        }

        void UpdateMaterialCode(const std::string &InCode);

        bool UseWorldOffset() { return bUseWorldOffset; }

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

        void SetParameterValue(const std::string &Name, FTexture *Texture)
        {
            Textures[Name] = Texture;
        }

        void SetParameterValue(const std::string &Name, FUniformBuffer *UniformBuffer)
        {
            UniformBuffers[Name] = UniformBuffer;
        }

    protected:

        std::string MaterialName;

        // FShaderCodeInitializer CodeInitializer;
        FShaderParserResult ParsedResult;

        bool bUseWorldOffset = false;

        FMaterialShaderMap ShaderMap;

        std::map<std::string, FTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        bool bShaderCompiled = false;

    };

    class FMaterialRenderProxy
    {
    public:
        FMaterialRenderProxy(FMaterial *InMaterial)
            : MaterialName(InMaterial->MaterialName)
            , ShaderMap(InMaterial->ShaderMap)
            , Textures(InMaterial->Textures)
            , UniformBuffers(InMaterial->UniformBuffers)
            , StencilRefValue(InMaterial->StencilRefValue)
            , RasterizerState(InMaterial->RasterizerState)
            , DepthStencilState(InMaterial->DepthStencilState)
            , BlendState(InMaterial->BlendState)
            , bShaderCompiled(InMaterial->bShaderCompiled)
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

        std::string MaterialName;

        FMaterialShaderMap ShaderMap;

        std::map<std::string, FTexture *> Textures;

        std::map<std::string, FUniformBuffer *> UniformBuffers;

        uint8 StencilRefValue = 0;

        FRasterizerStateInitializer RasterizerState;

        FDepthStencilStateInitializer DepthStencilState;

        FBlendStateInitializer BlendState;

        bool bShaderCompiled;
    };

}