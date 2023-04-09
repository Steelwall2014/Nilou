#include "ArrowComponent.h"
#include "StaticMeshResources.h"
#include "DynamicMeshResources.h"
#include "PrimitiveUtils.h"
#include "Common/ContentManager.h"
#include "Material.h"

constexpr float DEFAULT_SCREEN_SIZE	= 0.0025f;
constexpr float ARROW_SCALE			= 2.0f;
constexpr float ARROW_RADIUS_FACTOR	= 0.03f;
constexpr float ARROW_HEAD_FACTOR	= 0.2f;
constexpr float ARROW_HEAD_ANGLE	= 20.f;

namespace nilou {

    class FArrowSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FArrowSceneProxy(UArrowComponent *Component)
            : FPrimitiveSceneProxy(Component)
            , ArrowColor(Component->GetArrowColor())
            , ArrowSize(Component->GetArrowSize())
            , ArrowLength(Component->GetArrowLength())
            // , bIsScreenSizeScaled(Component->bIsScreenSizeScaled)
            // , ScreenSize(Component->ScreenSize)
        {
            Material = GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset");

            const float HeadAngle = glm::radians(ARROW_HEAD_ANGLE);
            const float DefaultLength = ArrowSize * ARROW_SCALE;
            const float TotalLength = ArrowSize * ArrowLength;
            const float HeadLength = DefaultLength * ARROW_HEAD_FACTOR;
            const float ShaftRadius = DefaultLength * ARROW_RADIUS_FACTOR;
            const float ShaftLength = (TotalLength - HeadLength * 0.5); // 10% overlap between shaft and head
            const vec3 ShaftCenter = vec3(0.5f * ShaftLength, 0, 0);

            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildConeVerts(HeadAngle, HeadAngle, -HeadLength, TotalLength, 32, ArrowColor, OutVerts, OutIndices);
            BuildCylinderVerts(ShaftCenter, vec3(0, 0, 1), vec3(0, 1, 0), vec3(1, 0, 0), ShaftRadius, 0.5f * ShaftLength, 16, ArrowColor, OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);

            VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);

            BeginInitResource(&IndexBuffer);
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    FMeshBatch Mesh;
                    Mesh.CastShadow = bCastShadow;
                    Mesh.MaterialRenderProxy = Material->GetResource()->CreateRenderProxy();
                    Mesh.Element.VertexFactory = &VertexFactory;
                    Mesh.Element.IndexBuffer = &IndexBuffer;
                    Mesh.Element.NumVertices = VertexBuffers.Positions.GetNumVertices();
                    Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
                    Collector.AddMesh(ViewIndex, Mesh);
                }
            }
        }

    private:
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        UMaterial *Material;

        vec3 Origin;
        vec4 ArrowColor;
        float ArrowSize;
        float ArrowLength;
        // bool bIsScreenSizeScaled;
        // float ScreenSize;
    };

    UArrowComponent::UArrowComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
    {
        ArrowColor = vec4(1, 0, 0, 1);
        ArrowSize = 1.0f;
        ArrowLength = ARROW_SCALE;
        // bIsScreenSizeScaled = false;
        // ScreenSize = DEFAULT_SCREEN_SIZE;
    }

    FPrimitiveSceneProxy *UArrowComponent::CreateSceneProxy()
    {
        return new FArrowSceneProxy(this);
    }


    FBoundingBox UArrowComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
        return FBoundingBox(vec3(0, -ARROW_SCALE, -ARROW_SCALE), vec3(ArrowSize * ArrowLength * 3.0f, ARROW_SCALE, ARROW_SCALE)).TransformBy(LocalToWorld);
    }

    void UArrowComponent::SetArrowColor(vec4 NewColor)
    {
        ArrowColor = NewColor;
        MarkRenderStateDirty();
    }

    void UArrowComponent::SetArrowSize(float NewSize)
    {
        ArrowSize = NewSize;
        UpdateBounds();
        MarkRenderStateDirty();
    }

    void UArrowComponent::SetArrowLength(float NewLength)
    {
        ArrowLength = NewLength;
        UpdateBounds();
        MarkRenderStateDirty();
    }
}