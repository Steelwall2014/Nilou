#pragma once
#include "Common/Maths.h"
#include "RHIDefinitions.h"

namespace nilou {

    struct FDynamicMeshVertex
    {
        FDynamicMeshVertex();
        
        FDynamicMeshVertex( const vec3& InPosition );

        FDynamicMeshVertex(const vec3& InPosition, const vec2& InTexCoord, const vec3& InColor);

        FDynamicMeshVertex(const vec3& InPosition,const vec4& InTangentX,const vec4& InTangentZ,const vec2& InTexCoord, const vec3& InColor);

        FDynamicMeshVertex(const vec3& InPosition, const vec3& LayerTexcoords, const vec2& WeightmapTexcoords);

        void SetTangents( const vec3& InTangentX, const vec3& InTangentY, const vec3& InTangentZ );

        vec3 GetTangentY() const;

        vec3 Position;
        vec2 TextureCoordinate[MAX_STATIC_TEXCOORDS];
        vec4 Tangent;
        vec4 Normal;
        vec3 Color;
    };


}