#include "LightingPassRendering.h"
#include "DefferedShadingSceneRenderer.h"

#include "RHIStaticStates.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FLightingPassVS, "/Shaders/GlobalShaders/LightingPassVertexShader.vert", EShaderFrequency::SF_Vertex, Global);
    IMPLEMENT_SHADER_TYPE(FLightingPassPS, "/Shaders/GlobalShaders/LightingPassPixelShader.frag", EShaderFrequency::SF_Pixel, Global);

    void FDefferedShadingSceneRenderer::RenderLightingPass(FDynamicRHI *RHICmdList)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *CameraInfo = Views[ViewIndex].ViewSceneInfo;
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            if (CameraInfo->Camera->IsMainCamera())
            {

                FRHIRenderPassInfo PassInfo(SceneTextures.FrameBuffer.get(), true);
                RHICmdList->RHIBeginRenderPass(PassInfo);
                {
                    
                    FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                    
                    FShaderPermutationParameters PermutationParametersPS(&FLightingPassPS::StaticType, 0);

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

                    for (auto &&LightInfo : Scene->AddedLightSceneInfos)
                    {
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
                            LightInfo->SceneProxy->LightUniformBufferRHI->GetRHI());

                        RHICmdList->RHIDrawArrays(0, 4);
                    }
                }
                RHICmdList->RHIEndRenderPass();

                break;
            }
        }
    }

}