#include "LightingPassRendering.h"
#include "Material.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FLightingPassVS, "/Shaders/GlobalShaders/LightingPassVertexShader.vert", EShaderFrequency::SF_Vertex, Global);
    IMPLEMENT_SHADER_TYPE(FLightingPassPS, "/Shaders/GlobalShaders/LightingPassPixelShader.frag", EShaderFrequency::SF_Pixel, Global);
    
    void FLightingPassPS::ModifyCompilationEnvironment(const FShaderPermutationParameters &Parameter, FShaderCompilerEnvironment &Environment)
    {
        // TODO
        FPermutationDomain Domain(Parameter.PermutationId);
        // int FrustumCount = Domain.Get<FDimensionFrustumCount>();
        // Environment.SetDefine("FrustumCount", FrustumCount);
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
            FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            FRHIRenderPassInfo PassInfo(SceneTextures.FrameBuffer.get(), CameraInfo->GetResolution(), true);

            RHICmdList->RHIBeginRenderPass(PassInfo);
            {
                
                for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
                {
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    int FrustumCount = Lights[LightIndex].ShadowMapTextures[ViewIndex].FrameBuffers.size();
                    FLightingPassPS::FPermutationDomain PermutationVector;
                    PermutationVector.Set<FLightingPassPS::FDimensionFrustumCount>(FrustumCount);
                    FShaderPermutationParameters PermutationParametersPS(&FLightingPassPS::StaticType, PermutationVector.ToDimensionValueId());

                    FShaderInstance *LightPassVS = GetContentManager()->GetGlobalShader(PermutationParametersVS);
                    FShaderInstance *LightPassPS = GetContentManager()->GetGlobalShader(PermutationParametersPS);
                    
                    FRHIGraphicsPipelineInitializer PSOInitializer;

                    PSOInitializer.VertexShader = LightPassVS;
                    PSOInitializer.PixelShader = LightPassPS;

                    PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Triangle_Strip;

                    FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                    
                    RHIDepthStencilStateRef DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI();
                    RHIRasterizerStateRef RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                    RHIBlendStateRef BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::CreateRHI();
                    RHIGetError();
                    RHICmdList->RHISetGraphicsPipelineState(PSO);
                    RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
                    RHICmdList->RHISetRasterizerState(RasterizerState.get());
                    RHICmdList->RHISetBlendState(BlendState.get());
                    RHIGetError();

                    auto &Light = Lights[LightIndex];
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "BaseColor", 
                        FRHISampler(SceneTextures.BaseColor));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "RelativeWorldSpacePosition", 
                        FRHISampler(SceneTextures.RelativeWorldSpacePosition));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "WorldSpaceNormal", 
                        FRHISampler(SceneTextures.WorldSpaceNormal));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "MetallicRoughness", 
                        FRHISampler(SceneTextures.MetallicRoughness));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "Emissive", 
                        FRHISampler(SceneTextures.Emissive));
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "ShadingModel", 
                        FRHISampler(SceneTextures.ShadingModel, RHITextureParams(ETextureFilters::TF_Nearest, ETextureFilters::TF_Nearest)));

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
                        CameraInfo->SceneProxy->GetViewUniformBuffer()->GetRHI());
                    RHIGetError();

                    RHICmdList->RHISetVertexBuffer(PSO, &PositionVertexInput);
                    RHIGetError();
                    RHICmdList->RHISetVertexBuffer(PSO, &UVVertexInput);
                    RHIGetError();
                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel, 
                        "FLightUniformBlock", 
                        Light.LightSceneInfo->SceneProxy->LightUniformBufferRHI->GetRHI());

                    auto UniformBuffer = FShadowMapUniformBuffers::Cast<CASCADED_SHADOWMAP_SPLIT_COUNT>(Light.ShadowMapUniformBuffers[ViewIndex]);
                    RHICmdList->RHISetShaderUniformBuffer(
                        PSO, EPipelineStage::PS_Pixel,
                        "FShadowMappingBlock", 
                        UniformBuffer->GetRHI());
                    RHITextureParams shadowMapSamplerParams;
                    shadowMapSamplerParams.Mag_Filter = ETextureFilters::TF_Nearest;
                    shadowMapSamplerParams.Min_Filter = ETextureFilters::TF_Nearest;
                    RHICmdList->RHISetShaderSampler(
                        PSO, EPipelineStage::PS_Pixel, 
                        "ShadowMaps", 
                        FRHISampler(Light.ShadowMapTextures[ViewIndex].DepthArray, shadowMapSamplerParams));

                    RHICmdList->RHIDrawArrays(0, 4);
                }
            }
            RHICmdList->RHIEndRenderPass();

        }
    }

}