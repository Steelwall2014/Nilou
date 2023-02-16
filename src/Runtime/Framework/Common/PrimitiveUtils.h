#pragma once

#include <vector>

#include "Common/Transform.h"
#include "Common/DynamicMeshResources.h"
#include "Platform.h"

namespace nilou {
    // vec3 unreal_vec3_xor(const vec3& this_, const vec3& V);
    vec3 GenerateYAxis(const vec4& XAxis, const vec4& ZAxis);
    float GetBasisDeterminantSign( const vec3& XAxis, const vec3& YAxis, const vec3& ZAxis );
    vec3 CalcConeVert(float Angle1, float Angle2, float AzimuthAngle);
    void BuildConeVerts(float Angle1, float Angle2, float Scale, float XOffset, uint32 NumSides, vec4 Color, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices);
    void BuildCylinderVerts(const vec3 &Base, const vec3 &XAxis, const vec3 &YAxis, const vec3 &ZAxis, float Radius, float HalfHeight, uint32 Sides, vec4 Color, std::vector<FDynamicMeshVertex> &OutVerts, std::vector<uint32> &OutIndices);
    void BuildCuboidVerts(float width, float length, float height, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices);
    void GetOrientedHalfSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, float StartAngle, float EndAngle, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices);
    void GetHalfSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices);
    void GetSphereMesh(const vec3& Center, const FRotator& Orientation, const vec3& Radii, int32 NumSides, int32 NumRings, std::vector<FDynamicMeshVertex>& OutVerts, std::vector<uint32>& OutIndices);
}