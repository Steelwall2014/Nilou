#version 460

layout (std140) uniform FShadowMappingParameters {
    dmat4 WorldToClip;
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
    vec4 ClipPosition = vec4(WorldToClip * dvec4(WorldPosition, 1));
    gl_Position = ClipPosition;
}