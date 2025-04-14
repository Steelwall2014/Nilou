#version 460 core
#include "../include/Common.glsl"

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
#if ENABLE_COLOR_COMPONENT
layout(location = 3) in vec4 COLOR;
#else
const vec4 COLOR = vec4(0);
#endif

layout(location = 4) in vec2 TEXCOORD_0;

layout (std140, set=VERTEX_FACTORY_SET_INDEX, binding=0) uniform FPrimitiveShaderParameters {
    dmat4 LocalToWorld;
	dmat4 ModelToLocal;
};

struct FVertexFactoryIntermediates
{
	dmat4 ModelToWorld;
};

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	VFIntermediates.ModelToWorld = LocalToWorld * ModelToLocal;
	return VFIntermediates;
}

dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	return dvec3(VFIntermediates.ModelToWorld * dvec4(POSITION, 1));
}

vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return mat3(transpose(inverse(mat3(VFIntermediates.ModelToWorld)))) * NORMAL;
}

vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return mat4(VFIntermediates.ModelToWorld) * TANGENT;
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return TEXCOORD_0;
}

vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	return COLOR;
}