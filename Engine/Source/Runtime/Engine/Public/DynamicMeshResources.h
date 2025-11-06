#pragma once
#include "Math/Maths.h"
#include "RHIDefinitions.h"

namespace nilou {

    struct FDynamicMeshVertex
    {
        FDynamicMeshVertex();
        
        FDynamicMeshVertex( const FVector3f& InPosition );

        FDynamicMeshVertex(const FVector3f& InPosition, const FVector2f& InTexCoord, const FVector3f& InColor);

        FDynamicMeshVertex(const FVector3f& InPosition,const FVector4f& InTangentX,const FVector4f& InTangentZ,const FVector2f& InTexCoord, const FVector3f& InColor);

        FDynamicMeshVertex(const FVector3f& InPosition, const FVector3f& LayerTexcoords, const FVector2f& WeightmapTexcoords);

        void SetTangents( const FVector3f& InTangentX, const FVector3f& InTangentY, const FVector3f& InTangentZ );

        FVector3f GetTangentY() const;

        FVector3f Position;
        FVector2f TextureCoordinate[MAX_STATIC_TEXCOORDS];
        FVector4f Tangent;
        FVector4f Normal;
        FVector3f Color;
    };


}