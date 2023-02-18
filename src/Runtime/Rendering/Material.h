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

    // std::vector<class FMaterialType *> &GetAllMaterialTypes();

    enum class EShadingModel
    {
        SM_Unlit,
        SM_DefaultLit,
        SM_Subsurface
    };

    // struct FMaterialPermutationParameters
    // {
    //     FMaterialType *Type;
    //     int32 PermutationId;
    //     FMaterialPermutationParameters(FMaterialType *InType=nullptr, int32 InPermutationId=0)
    //         : Type(InType)
    //         , PermutationId(InPermutationId)
    //     {}
    // };

    // struct FMaterialType : public FShaderTypeBase
    // {
    // public:
    //     friend class FShaderCompiler;

    //     FMaterialType() { }

    //     FMaterialType::FMaterialType(
    //         const std::string &InMaterialName, 
    //         const std::string &InMaterialFileName,
    //         std::function<bool(const FMaterialPermutationParameters&)> InShouldCompilePermutation,
    //         std::function<void(const FMaterialPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
    //         int32 InPermutationCount)
    //         : FShaderTypeBase(InMaterialName, InMaterialFileName, InPermutationCount)
    //         , ShouldCompilePermutation(InShouldCompilePermutation)
    //         , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
    //     {
    //         GetAllMaterialTypes().push_back(this);
    //     }
    // private:
    //     std::function<bool(const FMaterialPermutationParameters&)> ShouldCompilePermutation;
    //     std::function<void(const const FMaterialPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
    // };

    class FMaterial
    {
        friend class FShaderCompiler;
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

        void FillShaderBindings(FElementShaderBindings &OutBindings)
        { 
            for (auto &[Name, Texture] : Textures)
                OutBindings.SetElementShaderBinding(Name, Texture->GetSamplerRHI());
            for (auto &[Name, UniformBuffer] : UniformBuffers)
                OutBindings.SetElementShaderBinding(Name, UniformBuffer);
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

    };

    // class FDefaultMaterial : public FMaterial
    // {
    //     DECLARE_MATERIAL_TYPE()
    // public:
    //     FDefaultMaterial() { }
    //     FDefaultMaterial(
    //         const FRasterizerStateInitializer &InRasterizerState, 
    //         const FDepthStencilStateInitializer &InDepthStencilState) 
    //         : FMaterial(InRasterizerState, InDepthStencilState)
    //     { }

    // };

    // class FBasicMaterial : public FMaterial
    // {
    //     DECLARE_MATERIAL_TYPE()
    // public:
    //     virtual void INITSHADERBINDINGS() override
    //     {
    //     }
    // };

    // class FGLTFMaterial : public FMaterial
    // {
    //     DECLARE_MATERIAL_TYPE()
    // public:

    //     class FDimHasBaseColor : SHADER_PERMUTATION_BOOL("HAS_BASECOLOR");
    //     class FDimHasEmissive : SHADER_PERMUTATION_BOOL("HAS_EMISSIVE");
    //     class FDimHasNormal : SHADER_PERMUTATION_BOOL("HAS_NORMAL");
    //     class FDimHasOcclusion : SHADER_PERMUTATION_BOOL("HAS_OCCLUSION");
    //     class FDimHasMetallicRoughness : SHADER_PERMUTATION_BOOL("HAS_METALLICROUGHNESS");

    //     using FPermutationDomain = TShaderPermutationDomain<
    //                                     FDimHasBaseColor, 
    //                                     FDimHasEmissive, 
    //                                     FDimHasNormal, 
    //                                     FDimHasOcclusion,
    //                                     FDimHasMetallicRoughness>;

    //     FGLTFMaterial(
    //         const FRasterizerStateInitializer &InRasterizerState, 
    //         const FDepthStencilStateInitializer &InDepthStencilState) 
    //         : FMaterial(InRasterizerState, InDepthStencilState)
    //     { }

    //     /* Begin FMaterial Interface */
    //     virtual void FillShaderBindings(FElementShaderBindings &OutBindings) override
    //     { 
    //         if (BaseColorTexture)
    //             OutBindings.SetElementShaderBinding("baseColor", BaseColorTexture->GetSamplerRHI());
    //         if (EmissiveTexture)
    //             OutBindings.SetElementShaderBinding("emissiveMap", EmissiveTexture->GetSamplerRHI());
    //         if (NormalTexture)
    //             OutBindings.SetElementShaderBinding("normalMap", NormalTexture->GetSamplerRHI());
    //         if (OcclusionTexture)
    //             OutBindings.SetElementShaderBinding("occlusionMap", OcclusionTexture->GetSamplerRHI());
    //         if (MetallicRoughnessTexture)
    //             OutBindings.SetElementShaderBinding("roughnessMetallicMap", MetallicRoughnessTexture->GetSamplerRHI());
    //     }

    //     virtual int32 GetPermutationId() 
    //     { 
    //         FPermutationDomain PermutationVector;
    //         if (BaseColorTexture)
    //             PermutationVector.Set<FDimHasBaseColor>(true);
    //         if (EmissiveTexture)
    //             PermutationVector.Set<FDimHasEmissive>(true);
    //         if (NormalTexture)
    //             PermutationVector.Set<FDimHasNormal>(true);
    //         if (OcclusionTexture)
    //             PermutationVector.Set<FDimHasOcclusion>(true);
    //         if (MetallicRoughnessTexture)
    //             PermutationVector.Set<FDimHasMetallicRoughness>(true);
    //         return PermutationVector.ToDimensionValueId();
    //     }

    //     static void ModifyCompilationEnvironment(const FMaterialPermutationParameters& Parameters, FShaderCompilerEnvironment& Environment) 
    //     {
    //         FPermutationDomain PermutationVector;
    //         PermutationVector.FromDimensionValueId(Parameters.PermutationId);
    //         PermutationVector.ModifyCompilationEnvironment(Environment);
    //     }

    //     static bool ShouldCompilePermutation(const FMaterialPermutationParameters&) { return true; }

    //     /* End FMaterial Interface */

    //     std::shared_ptr<FTexture> BaseColorTexture;
    //     std::shared_ptr<FTexture> MetallicRoughnessTexture;
    //     std::shared_ptr<FTexture> NormalTexture;
    //     std::shared_ptr<FTexture> OcclusionTexture;
    //     std::shared_ptr<FTexture> EmissiveTexture;
    // };

    // class FColorerMaterial : public FMaterial
    // {
    //     DECLARE_MATERIAL_TYPE()
    // public:
    //     FColorerMaterial() { }
    //     FColorerMaterial(
    //         const FRasterizerStateInitializer &InRasterizerState, 
    //         const FDepthStencilStateInitializer &InDepthStencilState) 
    //         : FMaterial(InRasterizerState, InDepthStencilState)
    //     { }
    // };
}