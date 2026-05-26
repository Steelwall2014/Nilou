#include "RHIStaticStates.h"
#include "Shader.h"
#include "RenderGraphUtils.h"
#include "Renderer/DeferredShadingSceneRenderer.h"
#include "SkyAtmospherePassPixelShader.generated.h"

namespace nilou {

DECLARE_GLOBAL_SHADER(FSkyAtmospherePassPixelShader)
IMPLEMENT_SHADER_TYPE(FSkyAtmospherePassPixelShader, "/Shaders/Private/GlobalShaders/SkyAtmospherePassPixelShader.slang", "MainPS", EShaderFrequency::Pixel)
DEFINE_GRAPHICS_PIPELINE(FSkyAtmospherePipeline, FScreenQuadVertexShader, FSkyAtmospherePassPixelShader)

void FDeferredShadingSceneRenderer::RenderSkyAtmospherePass(RenderGraph& Graph)
{
    if (!Scene->SkyAtmosphere) return;
    FLightSceneProxy* FirstDirectionalLight = nullptr;
    for (FLightSceneInfo* LightInfo : Scene->AddedLightSceneInfos)
    {
        if (LightInfo->SceneProxy->LightType == ELightType::Directional)
        {
            FirstDirectionalLight = LightInfo->SceneProxy;
            break;
        }
    }
    if (!FirstDirectionalLight) return;
    for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
    {
        FSceneView& View = Views[ViewIndex];
        FSceneTextures& GBuffer = ViewSceneTextures[ViewIndex];

        RDGRenderTargets RenderTargets;
        RenderTargets.ColorAttachments[0] = { GBuffer.SceneColor->GetDefaultView(), ERenderTargetLoadAction::Load, ERenderTargetStoreAction::Store };
        const RHIRenderTargetLayout& RTLayout = RenderTargets.GetRenderTargetLayout();

        RHIGraphicsPipelineShaders* SkyAtmospherePipeline = GetGlobalGraphicsPipeline<FSkyAtmospherePipeline>();

        FGraphicsPipelineStateInitializer PSOInitializer;
        PSOInitializer.Shaders = *SkyAtmospherePipeline;
        PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
        PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
        PSOInitializer.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();
        PSOInitializer.VertexDeclaration = RDGGetScreenQuadVertexDeclaration();
        PSOInitializer.RTLayout = RTLayout;
        RHIGraphicsPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);

        RDGBuffer* ScreenQuadVertexBuffer = RDGGetScreenQuadVertexBuffer(Graph);
        RDGBuffer* ScreenQuadIndexBuffer = RDGGetScreenQuadIndexBuffer(Graph);

        auto LUTs = Graph.CreateParameterBlock<shader::SkyAtmosphereLUTs>("SkyPass SkyAtmosphereLUTs");
        LUTs->TransmittanceLUT = Scene->SkyAtmosphere->GetTransmittanceLUT();
        LUTs->ScatteringLUT = Scene->SkyAtmosphere->GetScatteringLUT();
        LUTs->linearSampler = TStaticSamplerState<SF_Trilinear>::GetRHI();
        Graph.UpdateParameterBlock(LUTs);

        auto Parameters = Graph.CreateParameterBlock<shader::SkyAtmospherePassPixelShaderParameters>("SkyPass Parameters");
        RDGTextureViewDesc Desc = CreateTextureViewDesc(GBuffer.DepthStencil);
        Desc.SubresourceRange = RHITextureSubresourceRange::MakeSingle(0, 0, 0); // depth plane for D32FS8
        Parameters->Depth = { Graph.CreateTextureView("SkyPass Depth", GBuffer.DepthStencil, Desc) };
        Parameters->SunDirection = -FirstDirectionalLight->Direction;
        Graph.UpdateParameterBlock(Parameters);

        auto AtmosphereParamBlock = Scene->SkyAtmosphere->GetAtmosphereParamBlock();

        RDGPassDesc PassDesc{NFormat("SkyAtmospherePass of view {}", ViewIndex)};

        Graph.AddGraphicsPass(
            PassDesc,
            RenderTargets,
            { ScreenQuadIndexBuffer },
            { ScreenQuadVertexBuffer },
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(AtmosphereParamBlock);
                Pass->AddParameterBlock(View.ViewUniformBuffer);
                Pass->AddParameterBlock(LUTs);
                Pass->AddParameterBlock(Parameters);
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindGraphicsPipelineState(PSO);

                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 AtmosphereSetIndex = PipelineLayout->GetSetIndex("ATMOSPHERE");
                int32 ViewParamsSetIndex = PipelineLayout->GetSetIndex("ViewParameters");
                int32 LUTsIndex = PipelineLayout->GetSetIndex("LUTs");
                int32 ParametersIndex = PipelineLayout->GetSetIndex("Parameters");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {
                        {AtmosphereSetIndex, AtmosphereParamBlock->GetDescriptorSet()->GetRHI()},
                        {ViewParamsSetIndex, View.ViewUniformBuffer->GetDescriptorSet()->GetRHI()},
                        {LUTsIndex, LUTs->GetDescriptorSet()->GetRHI()},
                        {ParametersIndex, Parameters->GetDescriptorSet()->GetRHI()}
                    },
                    EPipelineBindPoint::Graphics);

                RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);
                RHICmdList.DrawIndexed(6, 1, 0, 0, 0);
            });
    }

}

}