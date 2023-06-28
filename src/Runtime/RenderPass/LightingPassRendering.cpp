#include "LightingPassRendering.h"
#include "Material.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FLightingPassPS, "/Shaders/GlobalShaders/LightingPassPixelShader.frag", EShaderFrequency::SF_Pixel, Global);
    
    void FLightingPassPS::ModifyCompilationEnvironment(const FShaderPermutationParameters &Parameter, FShaderCompilerEnvironment &Environment)
    {
        FPermutationDomain Domain(Parameter.PermutationId);
        Domain.ModifyCompilationEnvironment(Environment);
        magic_enum::enum_for_each<EShadingModel>(
            [&Environment](EShadingModel ShadingModel){
                Environment.SetDefine(std::string(magic_enum::enum_name(ShadingModel)), (int)ShadingModel);
            });
    }

    void FDefferedShadingSceneRenderer::RenderLightingPass(FDynamicRHI *RHICmdList)
    {

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTexturesDeffered* SceneTextures = static_cast<FSceneTexturesDeffered*>(ViewInfo.SceneTextures);
            FRHIRenderPassInfo PassInfo(SceneTextures->LightPassFramebuffer.get(), ViewInfo.ScreenResolution, true);

            RHICmdList->RHIBeginRenderPass(PassInfo);
            {
                
                for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
                {
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    FShadowMapResource* ShadowMapResource = Lights[LightIndex].ShadowMapResources[ViewIndex];
                    int FrustumCount = ShadowMapResource->ShadowMapTexture.ShadowMapFramebuffers.size();
                    FLightingPassPS::FPermutationDomain PermutationVector;
                    PermutationVector.Set<FLightingPassPS::FDimensionFrustumCount>(FrustumCount);
                    FShaderPermutationParameters PermutationParametersPS(&FLightingPassPS::StaticType, PermutationVector.ToDimensionValueId());

                    FShaderInstance *LightPassVS = GetContentManager()->GetGlobalShader(PermutationParametersVS);
                    FShaderInstance *LightPassPS = GetContentManager()->GetGlobalShader(PermutationParametersPS);
                    
                    FGraphicsPipelineStateInitializer PSOInitializer;

                    PSOInitializer.VertexShader = LightPassVS->GetVertexShaderRHI();
                    PSOInitializer.PixelShader = LightPassPS->GetPixelShaderRHI();

                    PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_TriangleStrip;

                    PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI().get();
                    PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI().get();
                    PSOInitializer.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::CreateRHI().get();

                    static FRHIVertexInputList VertexInputList = {
                        PositionVertexInput,
                        UVVertexInput
                    };
                    PSOInitializer.VertexInputList = &VertexInputList;

                    PSOInitializer.BuildRenderTargetFormats(SceneTextures->LightPassFramebuffer.get());

                    FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                    
                    RHIGetError();
                    RHICmdList->RHISetGraphicsPipelineState(PSO);
                    RHIGetError();

                    auto &Light = Lights[LightIndex];
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "BaseColor", 
                        FRHISampler(SceneTextures->BaseColor));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "RelativeWorldSpacePosition", 
                        FRHISampler(SceneTextures->RelativeWorldSpacePosition));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "WorldSpaceNormal", 
                        FRHISampler(SceneTextures->WorldSpaceNormal));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "MetallicRoughness", 
                        FRHISampler(SceneTextures->MetallicRoughness));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "Emissive", 
                        FRHISampler(SceneTextures->Emissive));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "ShadingModel", 
                        FRHISampler(SceneTextures->ShadingModel, TStaticSamplerState<TF_Nearest, TF_Nearest>::CreateRHI().get()));

                    if (Scene->SkyAtmosphere)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "TransmittanceLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetTransmittanceLUT()));
                        RHIGetError();

                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "ScatteringRayleighLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetMultiScatteringLUT()));
                        RHIGetError();

                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "ScatteringMieLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetSingleScatteringMieLUT()));
                        RHIGetError();

                        RHICmdList->RHISetShaderUniformBuffer(
                            PSO, EPipelineStage::PS_Pixel, 
                            "AtmosphereParametersBlock", 
                            Scene->SkyAtmosphere->GetAtmosphereParametersBlock()->GetRHI());
                    }

                    RHIGetError();
                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel, 
                        "FViewShaderParameters", 
                        ViewInfo.ViewUniformBuffer->GetRHI());
                    RHIGetError();

                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel, 
                        "FLightUniformBlock", 
                        Light.LightUniformBuffer->GetRHI());

                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel,
                        "FShadowMappingBlock", 
                        ShadowMapResource->ShadowMapUniformBuffer.UniformBuffer->GetRHI());
                    RHITextureParams shadowMapSamplerParams(ETextureFilters::TF_Nearest, ETextureFilters::TF_Nearest);
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "ShadowMaps", 
                        FRHISampler(ShadowMapResource->ShadowMapTexture.DepthArray, TStaticSamplerState<TF_Nearest, TF_Nearest>::CreateRHI().get()));

                    RHICmdList->RHIDrawArrays(0, 4);
                }
            }
            RHICmdList->RHIEndRenderPass();

        }
    }

}