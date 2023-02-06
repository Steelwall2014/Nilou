#version 460

layout (std140) uniform FViewShaderParameters {
    mat4 WorldToView;
    mat4 ViewToClip;
    mat4 WorldToClip;
    mat4 ClipToWorld;
    vec3 CameraPosition;
    vec3 CameraDirection;
    ivec2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
};

//struct VS_Out {
//    mat3 TBN;
//    vec3 WorldPosition;
//    vec4 ClipPosition;
//    vec3 WorldNormal;
//    vec3 WorldTangent;
//    vec3 WorldBitangent;
//    vec4 Color;
//    vec2 TexCoords;
//};

//out vec3 WorldPosition;
//out vec3 WorldNormal;
//out vec2 UV;
//out mat3 TBN;
#include "../include/HandedCoordinateSystem.glsl"
//#include "../VertexFactories/StaticMeshVertexFactory.glsl"
//#include "../Materials/ColoredMaterial.glsl"

out VS_Out vs_out;
void main()
{
    FVertexFactoryIntermediates VFIntermediates = VertexFactoryIntermediates();
	vs_out.WorldNormal = VertexFactoryGetWorldNormal(VFIntermediates);
	vec4 tangent = VertexFactoryGetWorldTangent(VFIntermediates);
	vec3 bitanget = normalize(cross(vs_out.WorldNormal, tangent.xyz)) * tangent.w;
	vs_out.TBN = mat3(vec3(tangent), bitanget, vs_out.WorldNormal);
    vs_out.Color = VertexFactoryGetColor(VFIntermediates);
    vs_out.TexCoords = VertexFactoryGetTexCoord(VFIntermediates);
	vs_out.WorldPosition = VertexFactoryGetWorldPosition(VFIntermediates);
    vs_out.WorldPosition += MaterialGetWorldSpaceOffset(vs_out);
    vs_out.ClipPosition = WorldToClip * vec4(vs_out.WorldPosition, 1);
//    pos.x *= -1;
    gl_Position = ApplyHandedCoordinateSystem(vs_out.ClipPosition);
}

//#include "VertexFactories/StaticMeshVertexFactory.glsl"
//#include "include/Light.glsl"
//
//layout (std140) uniform FRenderBasePassShaderParameters {
//    mat4 ViewMatrix;
//    mat4 ProjectionMatrix;
//    mat4 VP;
//};
//
//struct VS_Out {
//    mat3 TBN;
//    vec3 WorldPosition;
//    vec3 WorldNormal;
//    vec3 WorldTangent;
//    vec3 WorldBitangent;
//    vec2 UV;
//};
//out VS_Out vs_out;
//
//
//void main()
//{
//    vs_out.WorldPosition = VertexFactoryGetWorldPosition();
//    vs_out.WorldNormal = VertexFactoryGetWorldNormal();
//    
//#ifdef SUPPORTS_TEXCOORD
//    vs_out.UV = VertexFactoryGetUV();
//#else
//    vs_out.UV = vec2(0);
//#endif
//
//#ifdef SUPPORTS_TANGENT
//    vec4 tangent = VertexFactoryGetWorldTangent();
//    vec3 bitanget = normalize(cross(vs_out.WorldNormal, tangent.xyz));
//    vs_out.TBN = mat3(vec3(tangent), bitanget, vs_out.WorldNormal);
//#else
//    vs_out.TBN = mat3(1);
//#endif
//
//    gl_Position = VP * vec4(vs_out.WorldPosition, 1);
//}