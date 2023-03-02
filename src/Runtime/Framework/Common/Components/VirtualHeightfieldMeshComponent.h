#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    // TODO: Finish this component
    UCLASS()
    class UVirtualHeightfieldMeshComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UVirtualHeightfieldMeshComponent(AActor *InOwner = nullptr);

        // 单个地形分段中四边形的数量（边长），一个分段即为地形渲染LOD过渡的单位
        // 默认缩放下一个四边形的大小为1*1
        uint32 NumQuadsPerSection = 16;

        // 单个地形节点中的分段数量（边长）。此数量与分段大小将决定各地形节点的大小。节点为渲染和剔除的基本单位。
        uint32 NumSectionsPerNode = 8;

        // X和Y方向的节点数量，这将作为最精细一级LOD的节点数
        uvec2 NodeCount{160, 160};

        void SetQuadsPerSection(uint32 InNumQuadsPerSection)
        {
            if (NumQuadsPerSection != InNumQuadsPerSection)
            {
                NumQuadsPerSection = InNumQuadsPerSection;
                MarkRenderStateDirty();
            }
        }

        void SetNodeCount(uvec2 InNodeCount)
        {
            if (NodeCount != InNodeCount)
            {
                NodeCount = InNodeCount;
                MarkRenderStateDirty();
            }
        }

        void SetSectionsPerNode(uint32 InNumSectionsPerNode)
        {
            if (NumSectionsPerNode != InNumSectionsPerNode)
            {
                NumSectionsPerNode = InNumSectionsPerNode;
                MarkRenderStateDirty();
            }
        }

        inline void SetMaterial(FMaterial *InMaterial)
        { 
            if (Material != InMaterial)
            {
                Material = InMaterial;
                MarkRenderStateDirty(); 
            }
        }

        inline FMaterial *GetMaterial() const { return Material; }

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.

    protected:

        FMaterial *Material;

    };

}