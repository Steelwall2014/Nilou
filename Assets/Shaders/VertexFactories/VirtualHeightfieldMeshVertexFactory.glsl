#version 460 core
layout(location = 0) in vec3 POSITION;
layout(location = 3) in vec4 COLOR;

layout(location = 4) in vec2 TEXCOORD_0;

#include "../VirtualHeightfieldMesh/VHM_RenderPatch.glsl"

layout(std430, binding=5) readonly buffer Patch_Buffer{
    RenderPatch patches[];
};

layout (std140) uniform FQuadTreeParameters {
    uvec2   NodeCount;
    uint    LODNum;
    uint    NumQuadsPerPatch;
    uint    NumPatchesPerNode;
};

layout (std140) uniform FPrimitiveShaderParameters {
    dmat4 LocalToWorld;
};

uniform sampler2D HeightfieldTexture;

uniform sampler2D NormalTexture;

uniform sampler2D TangentTexture;

struct FVertexFactoryIntermediates
{
	vec3 pos;
	vec2 Heightfield_UV;
	vec3 Normal;
	vec4 Tangent;
};

bool FixLODSeam(inout vec3 pos, float scale, RenderPatch current_patch)
{
	uint PatchGridSideNum = NumQuadsPerPatch+1;
	vec2 PatchOriginalGridMeterSize = vec2(1);
	uvec2 vertex_index = uvec2(gl_VertexID / PatchGridSideNum, gl_VertexID % PatchGridSideNum);
	bool on_edge = false;
	if (vertex_index.x == 0 || vertex_index.x == PatchGridSideNum-1 ||
		vertex_index.y == 0 || vertex_index.y == PatchGridSideNum-1)
		on_edge = true;
	if (vertex_index.x == 0 && current_patch.DeltaLod_x_neg > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_x_neg;	// �Ա���һ��patch�����patch�ļ�����
		uint modIndex = vertex_index.y % multiple;	
		if (modIndex != 0)	// �����0�Ļ��ǲ���Ҫ�޸Ķ���ģ���Ϊλ�ڸ��ֲ�patch�Ķ�����
		{
			pos.y += (multiple - modIndex) * PatchOriginalGridMeterSize.y * scale;
		}
	}
	if (vertex_index.x == PatchGridSideNum-1 && current_patch.DeltaLod_x_pos > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_x_pos;	// �Ա���һ��patch�����patch�ļ�����
		uint modIndex = vertex_index.y % multiple;	
		if (modIndex != 0)	// �����0�Ļ��ǲ���Ҫ�޸Ķ���ģ���Ϊλ�ڸ��ֲ�patch�Ķ�����
		{
			pos.y += (multiple - modIndex) * PatchOriginalGridMeterSize.y * scale;
		}
	}
	if (vertex_index.y == 0 && current_patch.DeltaLod_y_neg > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_y_neg;	// �Ա���һ��patch�����patch�ļ�����
		uint modIndex = vertex_index.x % multiple;	
		if (modIndex != 0)	// �����0�Ļ��ǲ���Ҫ�޸Ķ���ģ���Ϊλ�ڸ��ֲ�patch�Ķ�����
		{
			pos.x += (multiple - modIndex) * PatchOriginalGridMeterSize.x * scale;
		}
	}
	if (vertex_index.y == PatchGridSideNum-1 && current_patch.DeltaLod_y_pos > 0)
	{
		uint multiple = 1 << current_patch.DeltaLod_y_pos;	// �Ա���һ��patch�����patch�ļ�����
		uint modIndex = vertex_index.x % multiple;	
		if (modIndex != 0)	// �����0�Ļ��ǲ���Ҫ�޸Ķ���ģ���Ϊλ�ڸ��ֲ�patch�Ķ�����
		{
			pos.x += (multiple - modIndex) * PatchOriginalGridMeterSize.x * scale;
		}
	}
	return on_edge;
}

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	RenderPatch current_patch = patches[gl_InstanceID]; 
	float scale = pow(2, current_patch.lod);
	vec3 offset = vec3(current_patch.offset_x, current_patch.offset_y, 0);
	vec3 pos = POSITION;
	pos *= scale;
	pos += offset;
	FixLODSeam(pos, scale, current_patch);
	vec2 HeightTextureMeterSize = NumQuadsPerPatch * NumPatchesPerNode * NodeCount;
	vec2 HeightTexture_UV = pos.xy / HeightTextureMeterSize;
	VFIntermediates.pos = pos;
	VFIntermediates.Heightfield_UV = HeightTexture_UV;
	VFIntermediates.Normal = texture(NormalTexture, HeightTexture_UV).xyz;
	VFIntermediates.Tangent = texture(TangentTexture, HeightTexture_UV);
	return VFIntermediates;
}
dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	float height = texture(HeightfieldTexture, VFIntermediates.Heightfield_UV).r;
	VFIntermediates.pos.z += height;

	return dvec3(LocalToWorld * dvec4(VFIntermediates.pos, 1));
}

vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return mat3(transpose(inverse(mat3(LocalToWorld)))) * VFIntermediates.Normal;
}

vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return mat4(LocalToWorld) * VFIntermediates.Tangent;
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return TEXCOORD_0;
}

vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	RenderPatch current_patch = patches[gl_InstanceID]; 
	return vec4(current_patch.lod / 5.0);
}