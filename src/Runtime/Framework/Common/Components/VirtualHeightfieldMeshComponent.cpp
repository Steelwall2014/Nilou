#include "VirtualHeightfieldMeshComponent.h"

namespace nilou {

    class FVirtualHeightfieldMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FVirtualHeightfieldMeshSceneProxy(UVirtualHeightfieldMeshComponent *Component)
            : FPrimitiveSceneProxy(Component)
        {
            uint8 LodCount = 1;
            uvec2 NodeCount = Component->NodeCount;
            while (NodeCount.x % 2 == 0 && NodeCount.y % 2 == 0)
            {
                LodCount++;
                NodeCount.x /= 2;
                NodeCount.y /= 2;
            }
            LodParams.resize(LodCount);
            vec2 Lod0NodeMeterSize = vec2(Component->GetComponentScale()) * vec2(Component->NumSectionsPerNode) * vec2(Component->NumQuadsPerSection);
            int NodeNum = 0;
            for (int lod = 0; lod < LodCount; lod++)
            {
                WorldLodParam param;
                param.NodeMeterSize = Lod0NodeMeterSize * vec2(glm::pow(2, lod));
                param.NodeSideNum = Component->NodeCount / uvec2(glm::pow(2, lod));
                LodParams[lod] = param;
                NodeNum += param.NodeSideNum.x * param.NodeSideNum.y;
            }
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {

        }
    
        struct WorldLodParam
        {
            vec2 NodeMeterSize;
            uvec2 NodeSideNum;
            uint32 NodeDescriptionIndexOffset;	// 用来把一个node id(i, j, lod)转换到一维
        };
        struct RenderPatch
        {
            uvec4 DeltaLod;
            vec2 Offset;
            uint32 Lod;
            RenderPatch() : DeltaLod{0, 0, 0, 0}, Offset { 0, 0 }, Lod(0) {}
        };
        std::vector<WorldLodParam> LodParams;

	    FMaterialRenderProxy* Material;
        UTexture* HeightField;
        UTexture* HeightMinMaxTexture;
	    UTexture* LodBiasTexture;
	    UTexture* LodBiasMinMaxTexture;
        

    };



    UVirtualHeightfieldMeshComponent::UVirtualHeightfieldMeshComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
    {

    }

    FBoundingBox UVirtualHeightfieldMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
    {
        return FBoundingBox(vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f)).TransformBy(LocalToWorld);
    }

    FPrimitiveSceneProxy *UVirtualHeightfieldMeshComponent::CreateSceneProxy()
    {
        return new FVirtualHeightfieldMeshSceneProxy(this);
    }


}