#include "RenderPass.h"
#include "Material.h"

namespace nilou {

void BuildMeshDrawCommand(
    RenderGraph& Graph,
    const FVertexFactoryPermutationParameters &VFPermutationParameters,
    FMaterialRenderProxy *MaterialProxy,
    const FShaderPermutationParameters &PermutationParametersVS,
    const FShaderPermutationParameters &PermutationParametersPS,
    FRHIVertexDeclaration* VertexDeclaration,
    const FMeshBatchElement &Element,
    const RHIRenderTargetLayout &RTLayout,
    const FMeshDrawShaderBindings &ShaderBindings,
    RHIDepthStencilState* DepthStencilState,
    RHIRasterizerState* RasterizerState,
    RHIBlendState* BlendState,
    FMeshDrawCommand &OutMeshDrawCommand
)
{
    // Fill up the pipeline state initializer
    FGraphicsPipelineStateInitializer Initializer;
    FShaderInstance *VertexShader = MaterialProxy->GetShader(VFPermutationParameters, PermutationParametersVS);
    Initializer.VertexShader = VertexShader->GetVertexShaderRHI();
    FShaderInstance *PixelShader = MaterialProxy->GetShader(PermutationParametersPS);
    Initializer.PixelShader = PixelShader->GetPixelShaderRHI();
    Initializer.DepthStencilState = DepthStencilState;
    Initializer.RasterizerState = RasterizerState;
    Initializer.BlendState = BlendState;
    Initializer.VertexDeclaration = VertexDeclaration;
    Initializer.RTLayout = RTLayout;

    RHIGraphicsPipelineState* PipelineState = RHICreateGraphicsPipelineState(Initializer);
    auto PipelineDescriptorSetsLayout = PipelineState->GetPipelineLayout()->DescriptorSetLayouts;
    auto PushConstantRanges = PipelineState->GetPipelineLayout()->PushConstantRanges;

    for (auto& [Stage, PushConstantRange] : PushConstantRanges)
    {
        auto PushConstants = ShaderBindings.GetPushConstant(Stage);
        OutMeshDrawCommand.PushConstants[Stage] = PushConstants;
    }

    for (auto& [SetIndex, DescriptorSetLayout] : PipelineDescriptorSetsLayout)
    {
        std::string DescriptorSetName = NFormat("DescriptorSet_{}_{}_set{}", VertexShader->GetName(), PixelShader->GetName(), SetIndex);
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet(DescriptorSetName, DescriptorSetLayout);
        OutMeshDrawCommand.DescriptorSets[SetIndex] = DescriptorSet;
        for (auto& Binding : DescriptorSetLayout->Bindings)
        {
            std::string Name = Binding.Name;
            if (Binding.DescriptorType == EDescriptorType::UniformBuffer)
            {
                RDGBuffer* Buffer = ShaderBindings.GetBuffer(Name);
                DescriptorSet->SetUniformBuffer(Name, Buffer);
            }
            else if (Binding.DescriptorType == EDescriptorType::StorageBuffer)
            {
                RDGBuffer* Buffer = ShaderBindings.GetBuffer(Name);
                DescriptorSet->SetStorageBuffer(Name, Buffer);
            }
            else if (Binding.DescriptorType == EDescriptorType::CombinedImageSampler)
            {
                RDGTextureView* Texture = ShaderBindings.GetTexture(Name);
                DescriptorSet->SetSampler(Name, Texture);
            }
            else if (Binding.DescriptorType == EDescriptorType::StorageImage)
            {
                RDGTextureView* Texture = ShaderBindings.GetTexture(Name);
                DescriptorSet->SetStorageImage(Name, Texture);
            }
            else 
            {
                Ncheckf(false, "Unsupported descriptor type, Name={}, Type={}", Name.c_str(), magic_enum::enum_name(Binding.DescriptorType));
            }
        }
    }

    {
        OutMeshDrawCommand.PipelineState = PipelineState;
        OutMeshDrawCommand.VertexStreams = Element.VertexFactory->GetVertexInputStreams();
        OutMeshDrawCommand.IndexBuffer = Element.IndexBuffer->IndexBufferRDG.GetReference();
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