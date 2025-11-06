#if NILOU_ENABLE_3DTILES
#include "Cesium3DTileComponent.h"
#include "Cesium3DTileset.h"
#include "Common/Actor/GeoreferenceActor.h"
#include "StaticMeshResources.h"
#include "Material.h"

namespace nilou {

    using namespace Cesium3DTilesetSelection;

    const glm::dmat4 X_UP_TO_Z_UP = glm::dmat4( 0.0, 0.0, 1.0, 0.0,
                                                0.0, 1.0, 0.0, 0.0,
                                                -1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    const glm::dmat4 Y_UP_TO_Z_UP = glm::dmat4( 1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 1.0, 0.0,
                                                0.0, -1.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    class FCesium3DTileSceneProxy : public FPrimitiveSceneProxy
    {
    public:

        FCesium3DTileSceneProxy(UCesium3DTileComponent* Component)
            : FPrimitiveSceneProxy(Component)
            , Gltf(Component->Gltf)
        {
            PrimitiveUniformBuffer->Data.ModelToLocal = Component->ModelToLocal;
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {

            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    for (std::shared_ptr<UStaticMesh> StaticMesh : Gltf->StaticMeshes)
                    {
                        const FStaticMeshLODResources& LODModel = *StaticMesh->RenderData->LODResources[0];
                        for (int SectionIndex = 0; SectionIndex < LODModel.Sections.size(); SectionIndex++)
                        {
                            const FStaticMeshSection &Section = *LODModel.Sections[SectionIndex];
                            FMeshBatch Mesh;
                            Mesh.CastShadow = Section.bCastShadow;
                            FMeshBatchElement &Element = Mesh.Elements[0];
                            Element.VertexFactory = &Section.VertexFactory;
                            Element.IndexBuffer = &Section.IndexBuffer;
                            Element.NumVertices = Section.GetNumVertices();
                            Mesh.MaterialRenderProxy = StaticMesh->MaterialSlots[Section.MaterialIndex]->GetRenderProxy();
                            Collector.AddMesh(ViewIndex, Mesh);
                        }
                    }
                    
                }
            }
        }

        std::shared_ptr<GLTFParseResult> Gltf;
    };

    UCesium3DTileComponent::UCesium3DTileComponent()
        : Gltf(nullptr)
    {
    }

    void UCesium3DTileComponent::OnRegister()
    {
        
        dmat4 RtcCenterMatrix = dmat4(
            dvec4(1, 0, 0, 0), 
            dvec4(0, 1, 0, 0),
            dvec4(0, 0, 1, 0),
            dvec4(Tile->RtcCenter, 1)
        );
        dmat4 AxisTransform = dmat4(1);
        if (Tile->TileGltfUpAxis == ETileGltfUpAxis::Y)
            AxisTransform = Y_UP_TO_Z_UP;
        else if (Tile->TileGltfUpAxis == ETileGltfUpAxis::X)
            AxisTransform = X_UP_TO_Z_UP;
        dmat4 EcefToAbs = dmat4(1);
        if (ACesium3DTileset* Tileset = static_cast<ACesium3DTileset*>(GetOwner()))
        {
            if (Tileset->Georeference)
            {
                EcefToAbs = Tileset->Georeference->GetEcefToAbs();
            }
        }
        ModelToLocal = EcefToAbs * Tile->Transform * RtcCenterMatrix * AxisTransform;
        
        UPrimitiveComponent::OnRegister();
    }

    FBoundingBox UCesium3DTileComponent::CalcBounds(const FTransform& LocalToWorld) const
    {
        dmat4 EcefToAbs = glm::dmat4(1);
        if (ACesium3DTileset* Tileset = static_cast<ACesium3DTileset*>(GetOwner()))
        {
            if (Tileset->Georeference)
            {
                EcefToAbs = Tileset->Georeference->GetEcefToAbs();
            }
        }
        dvec3 center = LocalToWorld.TransformPosition(EcefToAbs * dvec4(Tile->BoundingVolume.Center, 1));
        dvec3 xDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * Tile->BoundingVolume.HalfAxes[0]);
        dvec3 yDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * Tile->BoundingVolume.HalfAxes[1]);
        dvec3 zDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * Tile->BoundingVolume.HalfAxes[2]);

        return FBoundingBox(center, xDirection, yDirection, zDirection);
    }

    FPrimitiveSceneProxy* UCesium3DTileComponent::CreateSceneProxy()
    {
        if (Gltf) {
            return new FCesium3DTileSceneProxy(this);
        }
        return nullptr;
    }

}
#endif