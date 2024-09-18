#pragma once

#include "MeshBatch.h"
#include "MeshPassProcessor.h"
#include "SceneView.h"
#include <vector>
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
);

}