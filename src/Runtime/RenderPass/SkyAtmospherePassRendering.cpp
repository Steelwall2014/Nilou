#include "SkyAtmospherePassRendering.h"
#include "DefferedShadingSceneRenderer.h"

#include "RHIStaticStates.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FSkyAtmospherePS, "/Shaders/GlobalShaders/SkyAtmospherePixelShader.frag", EShaderFrequency::SF_Pixel, Global)



    void FDefferedShadingSceneRenderer::RenderAtmospherePass(FDynamicRHI *RHICmdList)
    {
        if (Scene->SkyAtmosphere == nullptr) return;
        
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FCameraSceneInfo *CameraInfo = Views[ViewIndex].CameraSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            if (CameraInfo->Camera->IsMainCamera())
            {

                FRHIRenderPassInfo PassInfo(SceneTextures.FrameBuffer.get());
                RHICmdList->RHIBeginRenderPass(PassInfo);
                {
                    
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    FShaderPermutationParameters PermutationParametersPS(&FSkyAtmospherePS::StaticType, 0);

                    FShaderInstance *SkyAtmosphereVS = GetGlobalShaderInstance2(PermutationParametersVS);
                    FShaderInstance *SkyAtmospherePS = GetGlobalShaderInstance2(PermutationParametersPS);
                    
                    FRHIGraphicsPipelineInitializer PSOInitializer;

                    PSOInitializer.VertexShader = SkyAtmosphereVS;
                    PSOInitializer.PixelShader = SkyAtmospherePS;

                    PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Triangle_Strip;

                    FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                    
                    RHIDepthStencilStateRef DepthStencilState = TStaticDepthStencilState<false, CF_Always, true, CF_Equal>::CreateRHI();
                    RHIRasterizerStateRef RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                    RHIBlendStateRef BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::CreateRHI();
                    RHIGetError();
                    RHICmdList->RHISetGraphicsPipelineState(PSO);
                    RHICmdList->RHISetDepthStencilState(DepthStencilState.get(), 255);
                    RHICmdList->RHISetRasterizerState(RasterizerState.get());
                    RHICmdList->RHISetBlendState(BlendState.get());
                    RHIGetError();

                    for (auto &&LightInfo : Scene->AddedLightSceneInfos)
                    {
                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "TransmittanceLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetTransmittanceLUT()));
                        RHIGetError();

                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SingleScatteringRayleighLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetMultiScatteringLUT()));
                        RHIGetError();

                        RHICmdList->RHISetShaderSampler(
                            PSO, EPipelineStage::PS_Pixel, 
                            "SingleScatteringMieLUT", 
                            FRHISampler(Scene->SkyAtmosphere->GetSingleScatteringMieLUT()));
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

                        if (LightInfo->SceneProxy->LightType == ELightType::LT_Directional)
                        {
                            RHICmdList->RHISetShaderUniformBuffer(
                                PSO, EPipelineStage::PS_Pixel, 
                                "FLightUniformBlock", 
                                LightInfo->SceneProxy->LightUniformBufferRHI->GetRHI());
                            RHICmdList->RHIDrawArrays(0, 4);
                        }
                    }
                }
                RHICmdList->RHIEndRenderPass();

                break;
            }
        }
    }

}