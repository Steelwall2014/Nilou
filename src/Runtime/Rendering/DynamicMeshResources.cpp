#include "DynamicMeshResources.h"
#include "PrimitiveUtils.h"

namespace nilou {

    FDynamicMeshVertex::FDynamicMeshVertex():
        Position(FVector3f(0,0,0)),
        Tangent(FVector4f(1,0,0,1)),
        Normal(FVector4f(0,0,1,1)),
        Color(FVector3f(1,0,1)) 
    {

    }

    FDynamicMeshVertex::FDynamicMeshVertex( const FVector3f& InPosition ):
        Position(InPosition),
        Tangent(FVector4f(1,0,0,1)),
        Normal(FVector4f(0,0,1,1)),
        Color(FVector3f(1,0,1)) 
    {
        // basis determinant default to +1.0
        Tangent.w = 1;

        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            TextureCoordinate[i] = FVector2f(0);
        }
    }

    FDynamicMeshVertex::FDynamicMeshVertex(const FVector3f& InPosition, const FVector2f& InTexCoord, const FVector3f& InColor) :
        Position(InPosition),
        Tangent(FVector4f(1, 0, 0, 1)),
        Normal(FVector4f(0, 0, 1, 1)),
        Color(InColor)
    {
        // basis determinant default to +1.0
        Tangent.w = 1;

        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            TextureCoordinate[i] = InTexCoord;
        }
    }

    FDynamicMeshVertex::FDynamicMeshVertex(const FVector3f& InPosition,const FVector4f& InTangent,const FVector4f& InNormal,const FVector2f& InTexCoord, const FVector3f& InColor):
        Position(InPosition),
        Tangent(InTangent),
        Normal(InNormal),
        Color(InColor)
    {
        // basis determinant default to +1.0
        Tangent.w = 1.f;

        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            TextureCoordinate[i] = InTexCoord;
        }
    }

    FDynamicMeshVertex::FDynamicMeshVertex(const FVector3f& InPosition, const FVector3f& LayerTexcoords, const FVector2f& WeightmapTexcoords)
        : Position(InPosition)
        , Tangent(FVector4f(1, 0, 0, 1))
        , Normal(FVector4f(0, 0, 1, 1))
        , Color(FVector3f(1,0,1))
    {
        // Tangent.w contains the sign of the tangent basis determinant. Assume +1
        Tangent.w = 1.f;

        TextureCoordinate[0] = FVector2f(LayerTexcoords.x, LayerTexcoords.y);
        // TextureCoordinate[1] = FVector2f(LayerTexcoords.x, LayerTexcoords.y); // Z not currently set, so use Y
        // TextureCoordinate[2] = FVector2f(LayerTexcoords.y, LayerTexcoords.x); // Z not currently set, so use X
        // TextureCoordinate[3] = WeightmapTexcoords;
    };

    void FDynamicMeshVertex::SetTangents( const FVector3f& InTangent, const FVector3f& InTangentY, const FVector3f& InNormal )
    {
        Tangent = FVector4f(InTangent, 1);
        Normal = FVector4f(InNormal, 1);
        // store determinant of basis in w component of normal vector
        Tangent.w = GetBasisDeterminantSign(InTangent,InTangentY,InNormal);
    }

    FVector3f FDynamicMeshVertex::GetTangentY() const
    {
        return FVector3f(GenerateYAxis(Tangent, Normal));	//LWC_TODO: Precision loss
    };

}