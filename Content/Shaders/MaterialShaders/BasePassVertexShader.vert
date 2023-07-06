#version 460

#include "../include/ViewShaderParameters.glsl"

layout(location = 0) out VS_Out vs_out;
void main()
{
    FVertexFactoryIntermediates VFIntermediates = VertexFactoryIntermediates();
	vs_out.WorldNormal = VertexFactoryGetWorldNormal(VFIntermediates);
	vec4 tangent = VertexFactoryGetWorldTangent(VFIntermediates);
	vec3 bitanget = normalize(cross(vs_out.WorldNormal, tangent.xyz)) * tangent.w;
	vs_out.TBN = mat3(vec3(tangent), bitanget, vs_out.WorldNormal);
    vs_out.Color = VertexFactoryGetColor(VFIntermediates);
    vs_out.TexCoords = VertexFactoryGetTexCoord(VFIntermediates);
    dvec3 WorldPosition = VertexFactoryGetWorldPosition(VFIntermediates);
    vs_out.RelativeWorldPosition = vec3(WorldPosition - CameraPosition);
    vs_out.RelativeWorldPosition += MaterialGetWorldSpaceOffset(vs_out);
    vs_out.ClipPosition = RelWorldToClip * vec4(vs_out.RelativeWorldPosition, 1);
    gl_Position = vs_out.ClipPosition;
}