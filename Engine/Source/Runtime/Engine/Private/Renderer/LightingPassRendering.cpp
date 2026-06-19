#include "Renderer/LightingPassRendering.h"
#include "Materials/Material.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"
#include "LightingPassPixelShader.generated.h"
#include "Light.generated.h"
#include "ShadowMapping.generated.h"

namespace nilou {
    IMPLEMENT_SHADER_TYPE(FLightingPassPS, "/Shaders/Private/Lighting/LightingPassPixelShader.slang", "Main", EShaderFrequency::Pixel);
    DEFINE_GRAPHICS_PIPELINE(FLightingPassPipeline, FScreenQuadVertexShader, FLightingPassPS);

    void FDeferredShadingSceneRenderer::RenderLightingPass(RenderGraph& Graph)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& GBuffer = ViewSceneTextures[ViewIndex];

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = { GBuffer.SceneColor->GetDefaultView(), ERenderTargetLoadAction::Clear, ERenderTargetStoreAction::Store };
            RenderTargets.DepthStencilAttachment = { GBuffer.DepthStencil->GetDefaultView(), ERenderTargetLoadAction::Load, ERenderTargetStoreAction::Store };
            const RHIRenderTargetLayout& RTLayout = RenderTargets.GetRenderTargetLayout();

            for (int LightIndex = 0; LightIndex < VisibleLightInfos.size(); LightIndex++)
            {
                FVisibleLightInfo& LightInfo = VisibleLightInfos[LightIndex];
                FShadowMapResource& ShadowMapResource = LightInfo.ShadowMapResources[ViewIndex];
                int FrustumCount = ShadowMapResource.DepthViews.size();
                RHIGraphicsPipelineShaders* LightPassPipeline = GetGlobalGraphicsPipeline<FLightingPassPipeline>();

                FGraphicsPipelineStateInitializer PSOInitializer;
                PSOInitializer.Shaders = *LightPassPipeline;
                PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
                PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
                PSOInitializer.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();
                PSOInitializer.VertexDeclaration = RDGGetScreenQuadVertexDeclaration();
                PSOInitializer.RTLayout = RTLayout;
                RHIGraphicsPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);

                auto SceneTextureParams = Graph.CreateParameterBlock<shader::SceneTextures>("sceneTextures ParamBlock");
                SceneTextureParams->BaseColor = { GBuffer.BaseColor->GetDefaultView() };
                SceneTextureParams->RelativeWorldSpacePosition = { GBuffer.RelativeWorldSpacePosition->GetDefaultView() };
                SceneTextureParams->WorldSpaceNormal = { GBuffer.WorldSpaceNormal->GetDefaultView() };
                SceneTextureParams->MetallicRoughness = { GBuffer.MetallicRoughness->GetDefaultView() };
                SceneTextureParams->Emissive = { GBuffer.Emissive->GetDefaultView() };
                SceneTextureParams->ShadingModel = { GBuffer.ShadingModel->GetDefaultView(), TStaticSamplerState<SF_Point>::GetRHI() };
                Graph.UpdateParameterBlock(SceneTextureParams);

                TParameterBlock<shader::Light>* LightParams = LightInfo.LightSceneProxy->LightParams.GetReference();
                TParameterBlock<shader::ShadowMappingParameters>* ShadowParams = ShadowMapResource.ShadowMappingParameters;

                RDGBuffer* ScreenQuadVertexBuffer = RDGGetScreenQuadVertexBuffer(Graph);
                RDGBuffer* ScreenQuadIndexBuffer = RDGGetScreenQuadIndexBuffer(Graph);

                RDGPassDesc PassDesc{NFormat("LightingPass of view {} of light {}", ViewIndex, LightIndex)};
                PassDesc.bNeverCull = true;
                Graph.AddGraphicsPass(
                    PassDesc,
                    RenderTargets,
                    { ScreenQuadIndexBuffer },
                    { ScreenQuadVertexBuffer },
                    [=](FRDGPass* Pass)
                    {
                        Pass->AddParameterBlock(SceneTextureParams);
                        Pass->AddParameterBlock(View.ViewUniformBuffer);
                        Pass->AddParameterBlock(LightParams);
                        Pass->AddParameterBlock(ShadowParams);
                    },
                    [=](RHICommandList& RHICmdList)
                    {
                        RHICmdList.BindGraphicsPipelineState(PSO);

                        auto PipelineLayout = PSO->GetPipelineLayout();
                        int32 SceneTexturesSetIndex = PipelineLayout->GetSetIndex("sceneTextures");
                        int32 ViewParamsSetIndex = PipelineLayout->GetSetIndex("ViewParameters");
                        int32 LightSetIndex = PipelineLayout->GetSetIndex("light");
                        int32 ShadowMappingSetIndex = PipelineLayout->GetSetIndex("shadowMapping");
                        RHICmdList.BindDescriptorSets(
                            PipelineLayout,
                            {
                                {SceneTexturesSetIndex, SceneTextureParams->GetDescriptorSet()->GetRHI()},
                                {ViewParamsSetIndex, View.ViewUniformBuffer->GetDescriptorSet()->GetRHI()},
                                {LightSetIndex, LightParams->GetDescriptorSet()->GetRHI()},
                                {ShadowMappingSetIndex, ShadowParams->GetDescriptorSet()->GetRHI()}
                            },
                            EPipelineBindPoint::Graphics);

                        RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                        RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);
                        RHICmdList.DrawIndexed(6, 1, 0, 0, 0);
                    });
            }

        }
    }

}
