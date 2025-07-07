#ifndef VSCOMMON_H
#define VSCOMMON_H

#include "Macros.glsl"
#include "ViewShaderParameters.glsl"

#if FOR_INTELLISENSE
struct FVertexFactoryIntermediates
{
	dmat4 ModelToWorld;
};
FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates Intermediates;
	return Intermediates;
}
dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	return dvec3(0);
}
vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return vec3(0.0, 0.0, 1.0);
}
vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return vec4(0.0, 1.0, 0.0, 1.0);
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return vec2(0);
}
vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	return vec4(0);
}

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    return vec4(0);
}
vec3 MaterialGetEmissive(VS_Out vs_out)
{
    return vec3(0);
}
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    return vec3(0, 0, 1);
}
float MaterialGetRoughness(VS_Out vs_out)
{
    return 0.5;
}
float MaterialGetMetallic(VS_Out vs_out)
{
    return 0.5;
}
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
} 
#endif

#endif