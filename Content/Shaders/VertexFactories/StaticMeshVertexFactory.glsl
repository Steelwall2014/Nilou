#version 460 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
layout(location = 3) in vec4 COLOR;

layout(location = 4) in vec2 TEXCOORD_0;

layout (std140) uniform FPrimitiveShaderParameters {
    dmat4 LocalToWorld;
};

struct FVertexFactoryIntermediates
{
	mat4 FloatLocalToWorld;
};

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	return VFIntermediates;
}

dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	return dvec3(LocalToWorld * dvec4(POSITION, 1));
}

vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return mat3(transpose(inverse(mat3(LocalToWorld)))) * NORMAL;
}

vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return mat4(LocalToWorld) * TANGENT;
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return TEXCOORD_0;
}

vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	return COLOR;
}