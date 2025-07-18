#version 460

#include "../include/VertexShaderCommon.glsl"

#include "../include/ShadowMapShaderParameters.glsl"

layout (std140, set=SET_INDEX, binding=BINDING_INDEX) uniform FShadowMappingBlock {
    FShadowMappingParameters Frustums[FrustumCount];
};
layout (push_constant) uniform FShadowMapFrustumIndex {
    int FrustumIndex;
};
void main()
{
    VS_Out vs_out;
    FVertexFactoryIntermediates VFIntermediates = VertexFactoryIntermediates();
	vs_out.WorldNormal = VertexFactoryGetWorldNormal(VFIntermediates);
	vec4 tangent = VertexFactoryGetWorldTangent(VFIntermediates);
	vec3 bitanget = normalize(cross(vs_out.WorldNormal, tangent.xyz)) * tangent.w;
	vs_out.TBN = mat3(vec3(tangent), bitanget, vs_out.WorldNormal);
    vs_out.Color = VertexFactoryGetColor(VFIntermediates);
    vs_out.TexCoords = VertexFactoryGetTexCoord(VFIntermediates);
	dvec3 WorldPosition = VertexFactoryGetWorldPosition(VFIntermediates);
    WorldPosition += MaterialGetWorldSpaceOffset(vs_out);
    vs_out.RelativeWorldPosition = vec3(WorldPosition - CameraPosition);
    vec4 ClipPosition = vec4(Frustums[FrustumIndex].WorldToClip * dvec4(WorldPosition, 1));
    gl_Position = ClipPosition;
}