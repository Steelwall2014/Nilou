#include "RenderPass.h"
#include "Material.h"

namespace nilou {

void BuildMeshDrawCommand(
    const FVertexFactoryPermutationParameters &VFPermutationParameters,
    FMaterialRenderProxy *MaterialProxy,
    const FShaderPermutationParameters &PermutationParametersVS,
    const FShaderPermutationParameters &PermutationParametersPS,
    FRHIVertexDeclaration* VertexDeclaration,
    const FMeshBatchElement &Element,
    const RHIRenderTargetLayout &RTLayout,
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

#ifdef NILOU_DEBUG
    OutMeshDrawCommand.DebugVertexFactory = Element.VertexFactory;
    OutMeshDrawCommand.DebugMaterial = MaterialProxy;
#endif

}

}