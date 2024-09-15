#include "LightingPassRendering.h"
#include "Material.h"
#include "RHICommandList.h"

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

    void FDefferedShadingSceneRenderer::RenderLightingPass(RenderGraph& Graph)
    void FDeferredShadingSceneRenderer::RenderLightingPass(RenderGraph& Graph)
    {
        static FRHIVertexDeclaration* ScreenQuadVertexDeclaration = nullptr;
        if (!ScreenQuadVertexDeclaration)
        {
            FVertexDeclarationElementList Elements;
            Elements[0] = FVertexElement(0, 0, VET_Float4, 0, sizeof(float)*4);
            Elements[1] = FVertexElement(1, 0, VET_Float2, 1, sizeof(float)*2);
            ScreenQuadVertexDeclaration = RHICreateVertexDeclaration(Elements);
        }

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewInfo& ViewInfo = Views[ViewIndex];
            FSceneTextures SceneTextures = ViewSceneTextures[ViewIndex];
            // Build the render target layout of base pass
            RenderTargetLayout RTLayout;
            RTLayout.NumRenderTargetsEnabled = 2;
            RTLayout.RenderTargetFormats[FA_Color_Attachment0] = SceneTextures.SceneColor->Desc.Format;
            RTLayout.DepthStencilTargetFormat = SceneTextures.DepthStencil->Desc.Format;

            Graph.AddPass(
                [=](RDGPassBuilder& PassBuilder) 
                {
                    PassBuilder.RenderTarget(FA_Color_Attachment0, SceneTextures.SceneColor)
                               .RenderTarget(FA_Depth_Stencil_Attachment, SceneTextures.DepthStencil);
                },
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(SceneTextures->LightPassFramebuffer.get(), ViewInfo.ScreenResolution, true);
                    // RHICmdList.RHIBeginRenderPass(PassInfo);

                    for (int LightIndex = 0; LightIndex < Lights.size(); LightIndex++)
                    {
                        FShaderPermutationParameters PermutationParametersVS(&FScreenQuadVertexShader::StaticType, 0);
                        
                        FShadowMapResource ShadowMapResource = Lights[LightIndex].ShadowMapResources[ViewIndex];
                        int FrustumCount = ShadowMapResource.DepthViews.size();
                        FLightingPassPS::FPermutationDomain PermutationVector;
                        PermutationVector.Set<FLightingPassPS::FDimensionFrustumCount>(FrustumCount);
                        FShaderPermutationParameters PermutationParametersPS(&FLightingPassPS::StaticType, PermutationVector.ToDimensionValueId());

                        FShaderInstance *LightPassVS = GetGlobalShader(PermutationParametersVS);
                        FShaderInstance *LightPassPS = GetGlobalShader(PermutationParametersPS);
                        
                        FGraphicsPipelineStateInitializer PSOInitializer;

                        PSOInitializer.VertexShader = LightPassVS->GetVertexShaderRHI();
                        PSOInitializer.PixelShader = LightPassPS->GetPixelShaderRHI();

                        PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_TriangleStrip;

                        PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI().get();
                        PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI().get();
                        PSOInitializer.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::CreateRHI().get();

                        PSOInitializer.VertexDeclaration = ScreenQuadVertexDeclaration;

                        PSOInitializer.RTLayout = RTLayout;

                        FRHIPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);
                        
                        RHIGetError();
                        RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Graphics);
                        RHIGetError();

                        RHICmdList.BindVertexBuffer(0, PositionVertexBuffer.VertexBufferRDG->Resolve(), 0);
                        RHICmdList.BindVertexBuffer(1, UVVertexBuffer.VertexBufferRDG->Resolve(), 0);

                        PipelineDescriptorSets DescriptorSet{PSO};

                        DescriptorSet.SetSampler("BaseColor", FRHISampler(SceneTextures.BaseColor->Resolve()));
                        DescriptorSet.SetSampler("RelativeWorldSpacePosition", FRHISampler(SceneTextures.RelativeWorldSpacePosition->Resolve()));
                        DescriptorSet.SetSampler("WorldSpaceNormal", FRHISampler(SceneTextures.WorldSpaceNormal->Resolve()));
                        DescriptorSet.SetSampler("MetallicRoughness", FRHISampler(SceneTextures.MetallicRoughness->Resolve()));
                        DescriptorSet.SetSampler("Emissive", FRHISampler(SceneTextures.Emissive->Resolve()));
                        DescriptorSet.SetSampler("ShadingModel", FRHISampler(SceneTextures.ShadingModel->Resolve()));

                        if (Scene->SkyAtmosphere)
                        {
                            DescriptorSet.SetSampler("TransmittanceLUT", FRHISampler(Scene->SkyAtmosphere->GetTransmittanceLUT()));
                            DescriptorSet.SetSampler("ScatteringRayleighLUT", FRHISampler(Scene->SkyAtmosphere->GetMultiScatteringLUT()));
                            DescriptorSet.SetSampler("ScatteringMieLUT", FRHISampler(Scene->SkyAtmosphere->GetSingleScatteringMieLUT()));
                            DescriptorSet.SetUniformBuffer("AtmosphereParametersBlock", Scene->SkyAtmosphere->GetAtmosphereParametersBlock()->GetRHI());
                        }

                        DescriptorSet.SetUniformBuffer("FViewShaderParameters", ViewInfo.ViewUniformBuffer->GetRHI());
                        DescriptorSet.SetUniformBuffer("FLightUniformBlock", Lights[LightIndex].LightUniformBuffer->Resolve());
                        DescriptorSet.SetUniformBuffer("FShadowMappingBlock", ShadowMapResource.ShadowMapUniformBuffer->Resolve());
                        DescriptorSet.SetSampler("ShadowMaps", FRHISampler(ShadowMapResource.DepthArray->Resolve()));

                        RHICmdList.BindDescriptorSets(PSO->PipelineLayout.get(), DescriptorSet, EPipelineBindPoint::Graphics);
                        // auto &Light = Lights[LightIndex];
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "BaseColor", 
                        //     FRHISampler(SceneTextures->BaseColor));
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "RelativeWorldSpacePosition", 
                        //     FRHISampler(SceneTextures->RelativeWorldSpacePosition));
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "WorldSpaceNormal", 
                        //     FRHISampler(SceneTextures->WorldSpaceNormal));
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "MetallicRoughness", 
                        //     FRHISampler(SceneTextures->MetallicRoughness));
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "Emissive", 
                        //     FRHISampler(SceneTextures->Emissive));
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "ShadingModel", 
                        //     FRHISampler(SceneTextures->ShadingModel, TStaticSamplerState<TF_Nearest, TF_Nearest>::CreateRHI()));

                        // if (Scene->SkyAtmosphere)
                        // {
                        //     RHICmdList->RHISetShaderSampler(
                        //         PSO, EPipelineStage::PS_Pixel, 
                        //         "TransmittanceLUT", 
                        //         FRHISampler(Scene->SkyAtmosphere->GetTransmittanceLUT()));
                        //     RHIGetError();

                        //     RHICmdList->RHISetShaderSampler(
                        //         PSO, EPipelineStage::PS_Pixel, 
                        //         "ScatteringRayleighLUT", 
                        //         FRHISampler(Scene->SkyAtmosphere->GetMultiScatteringLUT()));
                        //     RHIGetError();

                        //     RHICmdList->RHISetShaderSampler(
                        //         PSO, EPipelineStage::PS_Pixel, 
                        //         "ScatteringMieLUT", 
                        //         FRHISampler(Scene->SkyAtmosphere->GetSingleScatteringMieLUT()));
                        //     RHIGetError();

                        //     RHICmdList->RHISetShaderUniformBuffer(
                        //         PSO, EPipelineStage::PS_Pixel, 
                        //         "AtmosphereParametersBlock", 
                        //         Scene->SkyAtmosphere->GetAtmosphereParametersBlock()->GetRHI());
                        // }

                        // RHIGetError();
                        // RHICmdList->RHISetShaderUniformBuffer(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "FViewShaderParameters", 
                        //     ViewInfo.ViewUniformBuffer->GetRHI());
                        // RHIGetError();

                        // RHICmdList->RHISetShaderUniformBuffer(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "FLightUniformBlock", 
                        //     Light.LightUniformBuffer->GetRHI());

                        // RHICmdList->RHISetShaderUniformBuffer(
                        //     PSO, EPipelineStage::PS_Pixel,
                        //     "FShadowMappingBlock", 
                        //     ShadowMapResource->ShadowMapUniformBuffer.UniformBuffer->GetRHI());
                        // RHITextureParams shadowMapSamplerParams(ETextureFilters::TF_Nearest, ETextureFilters::TF_Nearest);
                        // RHICmdList->RHISetShaderSampler(
                        //     PSO, EPipelineStage::PS_Pixel, 
                        //     "ShadowMaps", 
                        //     FRHISampler(ShadowMapResource->ShadowMapTexture.DepthArray, TStaticSamplerState<TF_Nearest, TF_Nearest>::CreateRHI()));

                        RHICmdList.DrawArrays(4, 1, 0, 0);
                    }
                    
                    RHICmdList.EndRenderPass();
                }
            );

        }
    }

}