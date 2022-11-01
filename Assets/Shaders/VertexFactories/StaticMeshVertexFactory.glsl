#version 460 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
layout(location = 3) in vec4 COLOR;

//#ifndef MATERIAL_TEXCOORD_NUM
//#define MATERIAL_TEXCOORD_NUM 1
//#endif

//#if MATERIAL_TEXCOORD_NUM >= 1 
layout(location = 4) in vec2 TEXCOORD_0;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 2 
//layout(location = 5) in vec2 TEXCOORD_1;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 3 
//layout(location = 6) in vec2 TEXCOORD_2;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 4 
//layout(location = 7) in vec2 TEXCOORD_3;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 5 
//layout(location = 8) in vec2 TEXCOORD_4;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 6 
//layout(location = 9) in vec2 TEXCOORD_5;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 7 
//layout(location = 10) in vec2 TEXCOORD_6;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 8 
//layout(location = 11) in vec2 TEXCOORD_7;
//#endif

//struct FVertexFactoryInterpolantsVSToPS
//{
//    vec4 Color;
//    vec2 TexCoords[1];
//};

layout (std140) uniform FPrimitiveShaderParameters {
    mat4 LocalToWorld;
};

struct FVertexFactoryIntermediates
{
	mat4 LeftHandedLocalToWorld;
};

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	VFIntermediates.LeftHandedLocalToWorld = LocalToWorld;
	return VFIntermediates;
}

vec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	return vec3(VFIntermediates.LeftHandedLocalToWorld * vec4(POSITION, 1));
}

vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return mat3(transpose(inverse(VFIntermediates.LeftHandedLocalToWorld))) * NORMAL;
}

vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return VFIntermediates.LeftHandedLocalToWorld * TANGENT;
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return TEXCOORD_0;
}

vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	return COLOR;
//#if MATERIAL_TEXCOORD_NUM >= 1 
//	Interpolants.Color = COLOR;
//    Interpolants.TexCoords[0] = TEXCOORD_0;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 2 
//    Interpolants.TexCoords[1] = TEXCOORD_1;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 3 
//    Interpolants.TexCoords[2] = TEXCOORD_2;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 4 
//    Interpolants.TexCoords[3] = TEXCOORD_3;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 5 
//    Interpolants.TexCoords[4] = TEXCOORD_4;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 6 
//    Interpolants.TexCoords[5] = TEXCOORD_5;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 7 
//    Interpolants.TexCoords[6] = TEXCOORD_6;
//#endif
//#if MATERIAL_TEXCOORD_NUM >= 8 
//    Interpolants.TexCoords[7] = TEXCOORD_7;
//#endif
}