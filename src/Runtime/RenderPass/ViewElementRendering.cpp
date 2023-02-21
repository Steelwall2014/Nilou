#include "ViewElementRendering.h"

#include "DefferedShadingSceneRenderer.h"
#include "RHIStaticStates.h"


namespace nilou {
    IMPLEMENT_SHADER_TYPE(FViewElementVS, "/Shaders/GlobalShaders/ViewElementVertexShader.vert", EShaderFrequency::SF_Vertex, Global)
    IMPLEMENT_SHADER_TYPE(FViewElementPS, "/Shaders/GlobalShaders/ViewElementPixelShader.frag", EShaderFrequency::SF_Pixel, Global)

    void FDefferedShadingSceneRenderer::RenderViewElementPass(FDynamicRHI *RHICmdList)
    {
        for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
        {
            FViewSceneInfo *ViewInfo = Views[ViewIndex].ViewSceneInfo;
            if (ViewInfo->PDI->LineElements.empty())
                continue;
            FStaticMeshVertexBuffer<vec3> PositionBuffer;
            FStaticMeshVertexBuffer<vec3> ColorBuffer;
            std::vector<vec3> Positions;
            std::vector<vec3> Colors;
            for (auto &&Line : ViewInfo->PDI->LineElements)
            {
                Positions.push_back(Line.Start);
                Positions.push_back(Line.End);
                Colors.push_back(Line.Color);
                Colors.push_back(Line.Color);
            }
            PositionBuffer.Init(Positions);
            ColorBuffer.Init(Colors);
            PositionBuffer.InitRHI();
            ColorBuffer.InitRHI();
            FRHIVertexInput PositionInput;
            PositionInput.VertexBuffer = PositionBuffer.VertexBufferRHI.get();
            PositionInput.Location = 0;
            PositionInput.Type = EVertexElementType::VET_Float3;
            PositionInput.Offset = 0;
            PositionInput.Stride = sizeof(vec3);

            FRHIVertexInput ColorInput;
            ColorInput.VertexBuffer = ColorBuffer.VertexBufferRHI.get();
            ColorInput.Location = 1;
            ColorInput.Type = EVertexElementType::VET_Float3;
            ColorInput.Offset = 0;
            ColorInput.Stride = sizeof(vec3);
            
            FSceneTextures &SceneTextures = Views[ViewIndex].SceneTextures;
            FRHIRenderPassInfo PassInfo(SceneTextures.FrameBuffer.get());
            RHICmdList->RHIBeginRenderPass(PassInfo);
            {
                FShaderPermutationParameters PermutationParametersVS(&FViewElementVS::StaticType, 0);
                FShaderPermutationParameters PermutationParametersPS(&FViewElementPS::StaticType, 0);

                FShaderInstance *ViewElementVS = FContentManager::GetContentManager().GetGlobalShader(PermutationParametersVS);
                FShaderInstance *ViewElementPS = FContentManager::GetContentManager().GetGlobalShader(PermutationParametersPS);
                
                FRHIGraphicsPipelineInitializer PSOInitializer;

                PSOInitializer.VertexShader = ViewElementVS;
                PSOInitializer.PixelShader = ViewElementPS;

                PSOInitializer.PrimitiveMode = EPrimitiveMode::PM_Lines;

                FRHIGraphicsPipelineState *PSO = RHICmdList->RHIGetOrCreatePipelineStateObject(PSOInitializer);
                
                RHIDepthStencilStateRef DepthStencilState = TStaticDepthStencilState<false>::CreateRHI();
                RHIRasterizerStateRef RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::CreateRHI();
                RHIBlendStateRef BlendState = TStaticBlendState<>::CreateRHI();
                RHIGetError();
                RHICmdList->RHISetGraphicsPipelineState(PSO);
                RHICmdList->RHISetDepthStencilState(DepthStencilState.get());
                RHICmdList->RHISetRasterizerState(RasterizerState.get());
                RHICmdList->RHISetBlendState(BlendState.get());
                RHIGetError();

                RHICmdList->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Vertex, 
                    "FViewShaderParameters", 
                    ViewInfo->SceneProxy->GetViewUniformBuffer()->GetRHI());

                RHIGetError();

                RHICmdList->RHISetVertexBuffer(PSO, &PositionInput);
                RHIGetError();
                RHICmdList->RHISetVertexBuffer(PSO, &ColorInput);
                RHIGetError();

                RHICmdList->RHIDrawArrays(0, Positions.size());
            }
            RHICmdList->RHIEndRenderPass();
            ViewInfo->PDI->LineElements.clear();
        }
    }

}