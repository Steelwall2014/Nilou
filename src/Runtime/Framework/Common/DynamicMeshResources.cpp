#include "DynamicMeshResources.h"
#include "Common/PrimitiveUtils.h"

namespace nilou {

    FDynamicMeshVertex::FDynamicMeshVertex():
        Position(vec3(0,0,0)),
        Tangent(vec4(1,0,0,1)),
        Normal(vec4(0,0,1,1)),
        Color(vec3(1,0,1)) 
    {

    }

    FDynamicMeshVertex::FDynamicMeshVertex( const vec3& InPosition ):
        Position(InPosition),
        Tangent(vec4(1,0,0,1)),
        Normal(vec4(0,0,1,1)),
        Color(vec3(1,0,1)) 
    {
        // basis determinant default to +1.0
        Tangent.w = 1;

        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            TextureCoordinate[i] = vec2(0);
        }
    }

    FDynamicMeshVertex::FDynamicMeshVertex(const vec3& InPosition, const vec2& InTexCoord, const vec3& InColor) :
        Position(InPosition),
        Tangent(vec4(1, 0, 0, 1)),
        Normal(vec4(0, 0, 1, 1)),
        Color(InColor)
    {
        // basis determinant default to +1.0
        Tangent.w = 1;

        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            TextureCoordinate[i] = InTexCoord;
        }
    }

    FDynamicMeshVertex::FDynamicMeshVertex(const vec3& InPosition,const vec4& InTangent,const vec4& InNormal,const vec2& InTexCoord, const vec3& InColor):
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

    FDynamicMeshVertex::FDynamicMeshVertex(const vec3& InPosition, const vec3& LayerTexcoords, const vec2& WeightmapTexcoords)
        : Position(InPosition)
        , Tangent(vec4(1, 0, 0, 1))
        , Normal(vec4(0, 0, 1, 1))
        , Color(vec3(1,0,1))
    {
        // Tangent.w contains the sign of the tangent basis determinant. Assume +1
        Tangent.w = 1.f;

        TextureCoordinate[0] = vec2(LayerTexcoords.x, LayerTexcoords.y);
        TextureCoordinate[1] = vec2(LayerTexcoords.x, LayerTexcoords.y); // Z not currently set, so use Y
        TextureCoordinate[2] = vec2(LayerTexcoords.y, LayerTexcoords.x); // Z not currently set, so use X
        TextureCoordinate[3] = WeightmapTexcoords;
    };

    void FDynamicMeshVertex::SetTangents( const vec3& InTangent, const vec3& InTangentY, const vec3& InNormal )
    {
        Tangent = vec4(InTangent, 1);
        Normal = vec4(InNormal, 1);
        // store determinant of basis in w component of normal vector
        Tangent.w = GetBasisDeterminantSign(InTangent,InTangentY,InNormal);
    }

    vec3 FDynamicMeshVertex::GetTangentY() const
    {
        return vec3(GenerateYAxis(Tangent, Normal));	//LWC_TODO: Precision loss
    };

}