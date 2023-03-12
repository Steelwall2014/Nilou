#version 460 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
layout(location = 3) in vec4 COLOR;

layout(location = 4) in vec2 TEXCOORD_0;

layout(std430, binding=3) readonly buffer NodeListBuffer{
    uvec4 NodeList[];
};

struct OceanLODParam
{
    vec2 NodeMeterSize;
    uvec2 NodeSideNum;
};
layout(std430, binding=4) readonly buffer LODParamsBuffer{
    OceanLODParam LODParams[];
};

layout (std140) uniform FPrimitiveShaderParameters {
    dmat4 LocalToWorld;
};

struct FVertexFactoryIntermediates
{
	vec3 pos;
};

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	uvec4 nodeLoc = NodeList[gl_InstanceID]; 
	float scale = pow(2, nodeLoc.z);

	vec2 offset = vec2(nodeLoc.x, nodeLoc.y) * vec2(LODParams[nodeLoc.z].NodeMeterSize.x, LODParams[nodeLoc.z].NodeMeterSize.y);
	vec3 pos = POSITION;
	pos *= scale;
	pos += vec3(offset, 0);
	VFIntermediates.pos = pos;
	return VFIntermediates;
}
dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	return dvec3(LocalToWorld * dvec4(VFIntermediates.pos, 1));
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
//	return vec4(VFIntermediates.MinMax/vec2(10), 0, 1);
}