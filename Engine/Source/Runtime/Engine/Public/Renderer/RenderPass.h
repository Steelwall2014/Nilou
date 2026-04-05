#pragma once

#include "MeshBatch.h"
#include "MeshPassProcessor.h"
#include "SceneView.h"
#include <vector>
namespace nilou {

void BuildMeshDrawCommand(
    RenderGraph& Graph,
    const FVertexFactoryPermutationParameters& VFParams,
    FMaterialRenderProxy *MaterialProxy,
    const FGraphicsPipelinePermutationParameters& PipelineParams,
    FRHIVertexDeclaration* VertexDeclaration,
    const FMeshBatchElement &Element,
    const RHIRenderTargetLayout &RTLayout,
    const FMeshDrawShaderBindings &ShaderBindings,
    RHIDepthStencilState* DepthStencilState,
    RHIRasterizerState* RasterizerState,
    RHIBlendState* BlendState,
    FMeshDrawCommand &OutMeshDrawCommand
);

}