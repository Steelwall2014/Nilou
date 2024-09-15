#include "ViewElementRendering.h"

#include "DeferredShadingSceneRenderer.h"
#include "RHIStaticStates.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FViewElementVS, "/Shaders/GlobalShaders/ViewElementVertexShader.vert", EShaderFrequency::SF_Vertex, Global)
    IMPLEMENT_SHADER_TYPE(FViewElementPS, "/Shaders/GlobalShaders/ViewElementPixelShader.frag", EShaderFrequency::SF_Pixel, Global)

    void FDeferredShadingSceneRenderer::RenderViewElementPass(RenderGraph& Graph)
    {
        // TODO: Implement this function
        
        // for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        // {
        //     FViewInfo& ViewInfo = Views[ViewIndex];
        //     if (ViewInfo.PDI.LineElements.empty())
        //         continue;
        //     FStaticMeshVertexBuffer<vec3> PositionBuffer;
        //     FStaticMeshVertexBuffer<vec3> ColorBuffer;
        //     std::vector<vec3> Positions;
        //     std::vector<vec3> Colors;
        //     for (auto &&Line : ViewInfo.PDI.LineElements)
        //     {
        //         Positions.push_back(Line.Start);
        //         Positions.push_back(Line.End);
        //         Colors.push_back(Line.Color);
        //         Colors.push_back(Line.Color);
        //     }
        //     PositionBuffer.Init(Positions);
        //     ColorBuffer.Init(Colors);
        //     PositionBuffer.InitResource();
        //     ColorBuffer.InitResource();
        //     FVertexElement PositionElement;
        //     PositionElement.AttributeIndex = 0;
        //     PositionElement.StreamIndex = 0;
        //     PositionElement.Offset = 0;
        //     PositionElement.Stride = 3 * sizeof(float);
        //     PositionElement.Type = EVertexElementType::VET_Float3;
        //     FVertexElement ColorElement;
        //     ColorElement.AttributeIndex = 1;
        //     ColorElement.StreamIndex = 1;
        //     ColorElement.Offset = 0;
        //     ColorElement.Stride = 3 * sizeof(float);
        //     ColorElement.Type = EVertexElementType::VET_Float3;
        //     FRHIVertexDeclaration* Declaration = FPipelineStateCache::GetOrCreateVertexDeclaration({ PositionElement, ColorElement} );
            
        //     FSceneTextures* SceneTextures = ViewInfo.SceneTextures;
        //     FRHIRenderPassInfo PassInfo(SceneTextures->LightPassFramebuffer.get(), ViewInfo.ScreenResolution);
        //     RHICmdList->RHIBeginRenderPass(PassInfo);
        //     {
        //         FShaderPermutationParameters PermutationParametersVS(&FViewElementVS::StaticType, 0);
        //         FShaderPermutationParameters PermutationParametersPS(&FViewElementPS::StaticType, 0);

        //         FShaderInstance *ViewElementVS = GetGlobalShader(PermutationParametersVS);
        //         FShaderInstance *ViewElementPS = GetGlobalShader(PermutationParametersPS);
                
        //         FGraphicsPipelineStateInitializer PSOInitializer;

        //         PSOInitializer.VertexShader = ViewElementVS->GetVertexShaderRHI();
        //         PSOInitializer.PixelShader = ViewElementPS->GetPixelShaderRHI();

        //         PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_LineList;

        //         PSOInitializer.DepthStencilState = TStaticDepthStencilState<false>::CreateRHI().get();
        //         PSOInitializer.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI().get();
        //         PSOInitializer.BlendState = TStaticBlendState<>::CreateRHI().get();

        //         PSOInitializer.VertexDeclaration = Declaration;

        //         FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
        //         RHIGetError();
        //         RHICmdList->RHISetGraphicsPipelineState(PSO);
        //         RHIGetError();

        //         RHICmdList->RHISetStreamSource(0, PositionBuffer.VertexBufferRHI.get(), 0);
        //         RHICmdList->RHISetStreamSource(1, ColorBuffer.VertexBufferRHI.get(), 0);

        //         RHICmdList->RHISetShaderUniformBuffer(
        //             PSO, EPipelineStage::PS_Vertex, 
        //             "FViewShaderParameters", 
        //             ViewInfo.ViewUniformBuffer->GetRHI());

        //         RHIGetError();

        //         RHICmdList->RHIDrawArrays(0, Positions.size());
        //     }
        //     RHICmdList->RHIEndRenderPass();
        //     ViewInfo.PDI.LineElements.clear();
        // }
    }

}