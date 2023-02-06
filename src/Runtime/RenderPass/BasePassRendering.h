#pragma once

#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "VertexFactory.h"
#include "RenderPass.h"

namespace nilou {
    DECLARE_MATERIAL_SHADER(FBasePassVS)
    DECLARE_MATERIAL_SHADER(FBasePassPS)
    // class FBasePassMeshPassProcessor : public FMeshPassProcessor
    // {
    // public: 
    //     virtual void BuildMeshDrawCommands(
    //         const FSceneView &View, 
    //         const std::vector<FMeshBatch> &MeshBatches, 
    //         std::vector<FMeshDrawCommand> &OutMeshDrawCommands);
    // };

}