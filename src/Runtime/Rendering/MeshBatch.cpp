#include "MeshBatch.h"
#include "Common/Components/PrimitiveComponent.h"

namespace nilou {
    void FMeshElementCollector::AddMesh(int32 ViewIndex, FMeshBatch &MeshBatch)
    {
        for (int32 ElementIndex = 0; ElementIndex < MeshBatch.Elements.size(); ElementIndex++)
        {
            FMeshBatchElement& MeshElement = MeshBatch.Elements[ElementIndex];

            if (!MeshElement.PrimitiveUniformBuffer)
            {
                MeshElement.PrimitiveUniformBuffer = PrimitiveSceneProxy->GetUniformBuffer();
            }
        }
        PerViewMeshBatches[ViewIndex].AddMesh(MeshBatch);
    }

    void FMeshElementCollector::AddBatchedLine(int32 ViewIndex, const FBatchedLine &MeshBatch)
    {
        PerViewPDI[ViewIndex].DrawLine(MeshBatch);
    }
}