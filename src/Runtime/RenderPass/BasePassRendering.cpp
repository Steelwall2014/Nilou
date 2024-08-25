#include "BasePassRendering.h"
#include "Common/Log.h"
#include "Material.h"
#include "DefferedShadingSceneRenderer.h"
#include "RHICommandList.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/MaterialShaders/BasePassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/MaterialShaders/BasePassPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    static void BuildMeshDrawCommand(
        const FVertexFactoryPermutationParameters &VFPermutationParameters,
        FMaterialRenderProxy *MaterialProxy,
        const FShaderPermutationParameters &PermutationParametersVS,
        const FShaderPermutationParameters &PermutationParametersPS,
        FRHIVertexDeclaration* VertexDeclaration,
        const FMeshBatchElement &Element,
        const RenderTargetLayout &RTLayout,
        std::vector<RDGDescriptorSet*> &OutDescriptorSets,
        FMeshDrawCommand &OutMeshDrawCommand
    )
    {
        // Fill up the pipeline state initializer
        FGraphicsPipelineStateInitializer Initializer;
        FShaderInstance *VertexShader = MaterialProxy->GetShader(VFPermutationParameters, PermutationParametersVS);
        Initializer.VertexShader = VertexShader->GetVertexShaderRHI();
        FShaderInstance *PixelShader = MaterialProxy->GetShader(PermutationParametersPS);
        Initializer.PixelShader = PixelShader->GetPixelShaderRHI();
        Initializer.DepthStencilState = TStaticDepthStencilState<true, CF_Equal>::CreateRHI().get();
        Initializer.RasterizerState = MaterialProxy->RasterizerState.get();
        Initializer.BlendState = MaterialProxy->BlendState.get();
        Initializer.VertexDeclaration = VertexDeclaration;
        Initializer.RTLayout = RTLayout;

        {
            for (auto& [SetIndex, DescriptorSet] : MaterialProxy->DescriptorSets)
            {
                OutMeshDrawCommand.ShaderBindings.SetDescriptorSet(SetIndex, DescriptorSet.get());
                OutDescriptorSets.push_back(DescriptorSet.get());
            }
            OutMeshDrawCommand.VertexStreams = Element.VertexFactory->GetVertexInputStreams();
            OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRDG.get();
            OutMeshDrawCommand.PipelineState = RHICreateGraphicsPipelineState(Initializer);
            OutMeshDrawCommand.NumInstances = Element.NumInstances;
            if (Element.IndirectArgsBuffer)
            {
                OutMeshDrawCommand.IndirectArgs.Buffer = Element.IndirectArgsBuffer;
                OutMeshDrawCommand.IndirectArgs.Offset = Element.IndirectArgsOffset;
            }
            else
            {
                OutMeshDrawCommand.VertexParams.BaseVertexIndex = Element.FirstIndex;
                OutMeshDrawCommand.VertexParams.NumVertices = Element.NumVertices;
            }
            OutMeshDrawCommand.StencilRef = MaterialProxy->StencilRefValue;
        }
    }

    void FDefferedShadingSceneRenderer::RenderBasePass(RenderGraph& Graph)
    {    
        std::vector<std::vector<RDGDescriptorSet*>> DescriptorSetsPerView(Views.size());
        std::vector<std::unordered_map<EFramebufferAttachment, RDGTexture*>> RenderTargetsPerView(Views.size());
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures& SceneTextures = ViewSceneTextures[ViewIndex];
            std::vector<FMeshBatch>& MeshBatches = ViewMeshBatches[ViewIndex];
            FParallelMeshDrawCommands DrawCommands;

            // Build the render target layout of base pass
            RenderTargetLayout RTLayout;
            RTLayout.NumRenderTargetsEnabled = 6;
            RTLayout.RenderTargetFormats[FA_Color_Attachment0] = SceneTextures.BaseColor->Desc.Format;
            RTLayout.RenderTargetFormats[FA_Color_Attachment1] = SceneTextures.RelativeWorldSpacePosition->Desc.Format;
            RTLayout.RenderTargetFormats[FA_Color_Attachment2] = SceneTextures.WorldSpaceNormal->Desc.Format;
            RTLayout.RenderTargetFormats[FA_Color_Attachment3] = SceneTextures.MetallicRoughness->Desc.Format;
            RTLayout.RenderTargetFormats[FA_Color_Attachment4] = SceneTextures.Emissive->Desc.Format;
            RTLayout.RenderTargetFormats[FA_Color_Attachment5] = SceneTextures.ShadingModel->Desc.Format;
            RTLayout.DepthStencilTargetFormat = SceneTextures.DepthStencil->Desc.Format;

            auto& RenderTargets = RenderTargetsPerView[ViewIndex];
            RenderTargets[FA_Color_Attachment0] = SceneTextures.BaseColor;
            RenderTargets[FA_Color_Attachment1] = SceneTextures.RelativeWorldSpacePosition;
            RenderTargets[FA_Color_Attachment2] = SceneTextures.WorldSpaceNormal;
            RenderTargets[FA_Color_Attachment3] = SceneTextures.MetallicRoughness;
            RenderTargets[FA_Color_Attachment4] = SceneTextures.Emissive;
            RenderTargets[FA_Color_Attachment5] = SceneTextures.ShadingModel;
            RenderTargets[FA_Depth_Stencil_Attachment] = SceneTextures.DepthStencil;

            auto& DescriptorSets = DescriptorSetsPerView[ViewIndex];
            RHIDescriptorSetLayout* VertexShaderLayout = FBasePassVS::GetDescriptorSetLayout(0, VERTEX_SHADER_SET_INDEX);
            RHIDescriptorSetLayout* PixelShaderLayout = FBasePassPS::GetDescriptorSetLayout(0, VERTEX_FACTORY_SET_INDEX);
            RDGDescriptorSet* VertexShaderDescriptorSet = Graph.CreateDescriptorSet(VertexShaderLayout);
            RDGDescriptorSet* PixelShaderDescriptorSet = Graph.CreateDescriptorSet(PixelShaderLayout);
            RDGBufferSRV* ViewUniformBufferSRV = Graph.CreateSRV(RDGBufferSRVDesc{View.ViewUniformBuffer, PF_Unknown});
            VertexShaderDescriptorSet->SetUniformBuffer("FViewShaderParameters", ViewUniformBufferSRV);
            DescriptorSets.push_back(VertexShaderDescriptorSet);
            DescriptorSets.push_back(PixelShaderDescriptorSet);

            for (FMeshBatch &Mesh : MeshBatches)
            {
                for (FMeshBatchElement& Element : Mesh.Elements)
                {
                    FVertexFactoryPermutationParameters VertexFactoryParams(Element.VertexFactory->GetType(), Element.VertexFactory->GetPermutationId());
                    FShaderPermutationParameters PermutationParametersVS(&FBasePassVS::StaticType, 0);
                    FShaderPermutationParameters PermutationParametersPS(&FBasePassPS::StaticType, 0);

                    FMeshDrawCommand MeshDrawCommand;
                    #ifdef NILOU_DEBUG
                    MeshDrawCommand.DebugVertexFactory = Element.VertexFactory;
                    MeshDrawCommand.DebugMaterial = Mesh.MaterialRenderProxy;
                    #endif
                    MeshDrawCommand.ShaderBindings.SetDescriptorSet(VERTEX_SHADER_SET_INDEX, VertexShaderDescriptorSet);
                    MeshDrawCommand.ShaderBindings.SetDescriptorSet(PIXEL_SHADER_SET_INDEX, PixelShaderDescriptorSet);

                    BuildMeshDrawCommand(
                        VertexFactoryParams,
                        Mesh.MaterialRenderProxy,
                        PermutationParametersVS,
                        PermutationParametersPS,
                        Element.VertexFactory->GetVertexDeclaration(),
                        Element,
                        RTLayout,
                        DescriptorSets,
                        MeshDrawCommand);

                    DrawCommands.AddMeshDrawCommand(MeshDrawCommand);
                    RHIGetError();
                }
                
            }
            ViewMeshDrawCommands.push_back(DrawCommands);
        }
        

         
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FSceneView& View = Views[ViewIndex];
            FSceneTextures SceneTextures = ViewSceneTextures[ViewIndex];
            RDGGraphicsPassDesc PassDesc;
            PassDesc.Name = "BasePass";
            PassDesc.RenderTargets = RenderTargetsPerView[ViewIndex];
            PassDesc.DescriptorSets = DescriptorSetsPerView[ViewIndex];
            Graph.AddGraphicsPass(
                PassDesc,
                [=](RHICommandList& RHICmdList)
                {
                    // FRHIRenderPassInfo PassInfo(SceneTextures->GeometryPassFramebuffer.get(), ViewInfo.ScreenResolution, true);
                    // RHICmdList.BeginRenderPass(PassInfo);
                    FParallelMeshDrawCommands &ViewCommands = ViewMeshDrawCommands[ViewIndex];
                    ViewCommands.DispatchDraw(RHICmdList);
                    // RHICmdList.EndRenderPass();
                }
            );
        }
    


    }
}