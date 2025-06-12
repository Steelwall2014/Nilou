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

    void FDeferredShadingSceneRenderer::RenderLightingPass(RenderGraph& Graph)
    {
        static FRHIVertexDeclaration* ScreenQuadVertexDeclaration = nullptr;
        if (!ScreenQuadVertexDeclaration)
        {
            FVertexDeclarationElementList Elements;
            Elements[0] = FVertexElement(0, 0, EVertexElementType::Float4, 0, sizeof(float)*4);
            Elements[1] = FVertexElement(1, 0, EVertexElementType::Float2, 1, sizeof(float)*2);
            ScreenQuadVertexDeclaration = RHICreateVertexDeclaration(Elements);
        }

        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = SceneTextures.SceneColor->GetDefaultView();
            RenderTargets.DepthStencilAttachment = SceneTextures.DepthStencil->GetDefaultView();
            const RHIRenderTargetLayout& RTLayout = RenderTargets.GetRenderTargetLayout();

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

                PSOInitializer.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::CreateRHI();
                PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                PSOInitializer.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::CreateRHI();

                PSOInitializer.VertexDeclaration = ScreenQuadVertexDeclaration;

                PSOInitializer.RTLayout = RTLayout;

                RHIGraphicsPipelineState *PSO = RHICreateGraphicsPipelineState(PSOInitializer);

                RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FLightingPassPS>(0, 0);
                DescriptorSet->SetSampler("BaseColor", SceneTextures.BaseColor->GetDefaultView());
                DescriptorSet->SetSampler("RelativeWorldSpacePosition", SceneTextures.RelativeWorldSpacePosition->GetDefaultView());
                DescriptorSet->SetSampler("WorldSpaceNormal", SceneTextures.WorldSpaceNormal->GetDefaultView());
                DescriptorSet->SetSampler("MetallicRoughness", SceneTextures.MetallicRoughness->GetDefaultView());
                DescriptorSet->SetSampler("Emissive", SceneTextures.Emissive->GetDefaultView());
                DescriptorSet->SetSampler("ShadingModel", SceneTextures.ShadingModel->GetDefaultView(), TStaticSamplerState<SF_Point>::GetRHI());

                if (Scene->SkyAtmosphere)
                {
                    DescriptorSet->SetSampler("TransmittanceLUT", Scene->SkyAtmosphere->GetTransmittanceLUT());
                    DescriptorSet->SetSampler("ScatteringRayleighLUT", Scene->SkyAtmosphere->GetMultiScatteringLUT());
                    DescriptorSet->SetSampler("ScatteringMieLUT", Scene->SkyAtmosphere->GetSingleScatteringMieLUT());
                    DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", Scene->SkyAtmosphere->GetAtmosphereParametersBlock());
                }

                DescriptorSet->SetUniformBuffer("FViewShaderParameters", View.ViewUniformBuffer);
                DescriptorSet->SetUniformBuffer("FLightUniformBlock", Lights[LightIndex].LightUniformBuffer);
                DescriptorSet->SetUniformBuffer("FShadowMappingBlock", ShadowMapResource.ShadowMapUniformBuffer);
                DescriptorSet->SetSampler("ShadowMaps", ShadowMapResource.DepthArray->GetDefaultView());

                RDGPassDesc PassDesc{NFormat("LightingPass of view {} of light {}", ViewIndex, LightIndex)};
                PassDesc.bNeverCull = true;
                Graph.AddGraphicsPass(
                    PassDesc, 
                    RenderTargets,
                    { },
                    { PositionVertexBuffer.VertexBufferRDG, UVVertexBuffer.VertexBufferRDG },
                    { DescriptorSet }, 
                    [=](RHICommandList& RHICmdList)
                    {
                        RHIGetError();
                        RHICmdList.BindGraphicsPipelineState(PSO);
                        RHIGetError();

                        RHICmdList.BindVertexBuffer(0, PositionVertexBuffer.VertexBufferRDG->GetRHI(), 0);
                        RHICmdList.BindVertexBuffer(1, UVVertexBuffer.VertexBufferRDG->GetRHI(), 0);

                        RHICmdList.BindDescriptorSets(PSO->PipelineLayout, { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Graphics);

                        RHICmdList.DrawArrays(4, 1, 0, 0);
                    });
            }

        }
    }

}