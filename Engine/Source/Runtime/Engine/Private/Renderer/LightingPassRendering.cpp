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
            RenderTargets.DepthStencilAttachment = { GBuffer.DepthStencil->GetDefaultView(), ERenderTargetLoadAction::Load, ERenderTargetStoreAction::NoAction };
            const RHIRenderTargetLayout& RTLayout = RenderTargets.GetRenderTargetLayout();

            for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
            {
                FShadowMapResource ShadowMapResource = Lights[LightIndex].ShadowMapResources[ViewIndex];
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

                FLightSceneProxy* LightProxy = Lights[LightIndex].LightSceneProxy;
                auto LightParams = Graph.CreateParameterBlock<shader::Light>("light ParamBlock");
                LightParams->distAttenCurve.params = *reinterpret_cast<const FVector4f*>(&LightProxy->DistAttenCurve.u);
                LightParams->distAttenCurve.scale = LightProxy->DistAttenCurve.scale;
                LightParams->angleAttenCurve.params = *reinterpret_cast<const FVector4f*>(&LightProxy->AngleAttenCurve.u);
                LightParams->angleAttenCurve.scale = LightProxy->AngleAttenCurve.scale;
                LightParams->position = FVector3f(LightProxy->Position);
                LightParams->intensity = LightProxy->LightIntensity;
                LightParams->direction = LightProxy->Direction;
                LightParams->castShadow = LightProxy->bCastShadow;
                LightParams->type = (int)LightProxy->LightType;
                Graph.UpdateParameterBlock(LightParams);

                auto ShadowParams = Graph.CreateParameterBlock<shader::ShadowMappingParameters>("shadowMapping ParamBlock");
                ShadowParams->FrustumCount = FrustumCount;
                ShadowParams->shadowMaps = { ShadowMapResource.DepthArray->GetDefaultView(), TStaticSamplerState<SF_Point>::GetRHI() };
                ShadowParams->frustums = ShadowMapResource.ShadowMapUniformBuffer;
                Graph.UpdateParameterBlock(ShadowParams);

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
                        RHIGetError();
                        RHICmdList.BindGraphicsPipelineState(PSO);
                        RHIGetError();

                        auto PipelineLayout = PSO->GetPipelineLayout();
                        int32 SceneTexturesSetIndex = PipelineLayout->GetSetIndex("sceneTextures");
                        int32 ViewParamsSetIndex = PipelineLayout->GetSetIndex("ViewParameters");
                        int32 LightSetIndex = PipelineLayout->GetSetIndex("light");
                        int32 ShadowMappingSetIndex = PipelineLayout->GetSetIndex("shadowMapping");
                        RHICmdList.BindDescriptorSets(
                            PipelineLayout,
                            {
                                {SceneTexturesSetIndex, SceneTextureParams->DescriptorSet->GetRHI()},
                                {ViewParamsSetIndex, View.ViewUniformBuffer->DescriptorSet->GetRHI()},
                                {LightSetIndex, LightParams->DescriptorSet->GetRHI()},
                                {ShadowMappingSetIndex, ShadowParams->DescriptorSet->GetRHI()}
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
