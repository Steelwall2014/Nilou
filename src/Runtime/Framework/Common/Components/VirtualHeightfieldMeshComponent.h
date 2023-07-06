#pragma once
#include "PrimitiveComponent.h"
#include "Material.h"

namespace nilou {

    class FVHMVertexFactory : public FVertexFactory
    {
        DECLARE_VERTEX_FACTORY_TYPE(FVHMVertexFactory)
    public:

        FVHMVertexFactory() { }

        struct FDataType
        {
            FVertexStreamComponent PositionComponent;
            
            FVertexStreamComponent TexCoordComponent[MAX_STATIC_TEXCOORDS];

            FVertexStreamComponent ColorComponent;
        };

	    void SetData(const FDataType& InData);

        virtual void InitVertexFactory() override;

        static bool ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters);

        static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);
        
    protected:
        FDataType Data;

    };

    class NCLASS UVirtualHeightfieldMeshComponent : public UPrimitiveComponent
    {
        GENERATED_BODY()
    public:

        UVirtualHeightfieldMeshComponent();

        // 单个地形分段中四边形的数量（边长），一个分段即为地形渲染LOD过渡的单位
        // 默认缩放下一个四边形的大小为1*1
        uint32 NumQuadsPerPatch = 16;

        // 单个地形节点中的分段数量（边长）。此数量与分段大小将决定各地形节点的大小。节点为渲染和剔除的基本单位。
        uint32 NumPatchesPerNode = 8;

        // X和Y方向的节点数量，这将作为最精细一级LOD的节点数
        uvec2 NodeCount{160, 160};

        void SetQuadsPerPatch(uint32 InNumQuadsPerPatch)
        {
            if (NumQuadsPerPatch != InNumQuadsPerPatch)
            {
                NumQuadsPerPatch = InNumQuadsPerPatch;
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

        void SetPatchsPerNode(uint32 InNumPatchsPerNode)
        {
            if (NumPatchesPerNode != InNumPatchsPerNode)
            {
                NumPatchesPerNode = InNumPatchsPerNode;
                MarkRenderStateDirty();
            }
        }

        inline void SetMaterial(UMaterial *InMaterial)
        { 
            if (Material != InMaterial)
            {
                Material = InMaterial;
                MarkRenderStateDirty(); 
            }
        }

        inline void SetHeightfieldTexture(UVirtualTexture *InHeightfieldTexture)
        { 
            if (HeightfieldTexture != InHeightfieldTexture)
            {
                HeightfieldTexture = InHeightfieldTexture;
                MarkRenderStateDirty(); 
            }
        }

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.

        UMaterial *Material = nullptr;

        UVirtualTexture *HeightfieldTexture = nullptr;

        float HeightfieldMin = 0, HeightfieldMax = 50;
        
        int NumMaxRenderingNodes = 500;

    };
    
    BEGIN_UNIFORM_BUFFER_STRUCT(FCreateNodeListBlock)
        SHADER_PARAMETER(uint32, MaxLOD)
        SHADER_PARAMETER(uint32, PassLOD)
        SHADER_PARAMETER(float, ScreenSizeDenominator)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FCreatePatchBlock)
        SHADER_PARAMETER(uvec2, LodTextureSize)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FQuadTreeParameters)
        SHADER_PARAMETER(uvec2, NodeCount)
        SHADER_PARAMETER(uint32, LODNum)
        SHADER_PARAMETER(uint32, NumQuadsPerPatch)
        SHADER_PARAMETER(uint32, NumPatchesPerNode)
        SHADER_PARAMETER(uint32, NumHeightfieldTextureMipmap)
    END_UNIFORM_BUFFER_STRUCT()
    
    BEGIN_UNIFORM_BUFFER_STRUCT(FBuildNormalTangentBlock)
        SHADER_PARAMETER(uint32, HeightfieldWidth)
        SHADER_PARAMETER(uint32, HeightfieldHeight)
        SHADER_PARAMETER(vec2, PixelMeterSize)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FBuildMinMaxBlock)
        SHADER_PARAMETER(uvec2, Offset)
    END_UNIFORM_BUFFER_STRUCT()

}