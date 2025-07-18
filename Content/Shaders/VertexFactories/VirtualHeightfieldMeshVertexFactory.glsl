#version 460 core
layout(location = 0) in vec3 POSITION;
layout(location = 3) in vec4 COLOR;

layout(location = 4) in vec2 TEXCOORD_0;

#include "../include/Macros.glsl"
#include "../VirtualHeightfieldMesh/VHM_RenderPatch.glsl"

layout(std430, binding=8) readonly buffer Patch_Buffer{
    RenderPatch patches[];
};

layout (std140, binding=9) uniform FQuadTreeParameters {
    uvec2   NodeCount;
    uint    LODNum;
    uint    NumQuadsPerPatch;
    uint    NumPatchesPerNode;
	uint	NumHeightfieldTextureMipmap;
};

layout (std140, binding=10) uniform FBuildNormalTangentBlock {
    uint HeightfieldWidth;
    uint HeightfieldHeight;
    vec2 PixelMeterSize;
};

layout (std140, binding=11) uniform FPrimitiveShaderParameters {
    dmat4 LocalToWorld;
	dmat4 ModelToLocal;
};

#include "../include/Macros.glsl"
layout (binding=1) uniform sampler2D HeightfieldTexture;

//uniform sampler2D MinMaxMap;

//uniform sampler2D NormalTexture;

//uniform sampler2D TangentTexture;

struct FVertexFactoryIntermediates
{
	vec3 pos;
	vec2 Heightfield_UV;
	vec3 Normal;
	vec4 Tangent;
	uint MipmapLevel;
	dmat4 ModelToWorld;
};

bool FixLODSeam(inout vec3 pos, float scale, RenderPatch current_patch)
{
	uint PatchGridSideNum = NumQuadsPerPatch+1;
	vec2 PatchOriginalGridMeterSize = vec2(1);
	uvec2 vertex_index = uvec2(gl_VertexIndex / PatchGridSideNum, gl_VertexIndex % PatchGridSideNum);
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

void CalcNormalTangent(ivec2 id, uint MipmapLevel, out vec3 Normal, out vec4 Tangent)
{
    vec2 uvX1 = (vec2(id.x - 1, id.y) + vec2(0.5)) / vec2(HeightfieldWidth, HeightfieldHeight);
    vec2 uvX2 = (vec2(id.x + 1, id.y) + vec2(0.5)) / vec2(HeightfieldWidth, HeightfieldHeight);
    vec2 uvY1 = (vec2(id.x, id.y - 1) + vec2(0.5)) / vec2(HeightfieldWidth, HeightfieldHeight);
    vec2 uvY2 = (vec2(id.x, id.y + 1) + vec2(0.5)) / vec2(HeightfieldWidth, HeightfieldHeight);

    float x1_displace = textureLod(HeightfieldTexture, uvX1, MipmapLevel).r;
    float x2_displace = textureLod(HeightfieldTexture, uvX2, MipmapLevel).r;
    float y1_displace = textureLod(HeightfieldTexture, uvY1, MipmapLevel).r;
    float y2_displace = textureLod(HeightfieldTexture, uvY2, MipmapLevel).r;

    vec3 tangentX = vec3(2*PixelMeterSize.x, 0, x2_displace-x1_displace);
    vec3 tangentY = vec3(0, 2*PixelMeterSize.y, y2_displace-y1_displace);

    Normal = normalize(cross(tangentX, tangentY));
	Tangent = vec4(normalize(tangentX), -1);
}

FVertexFactoryIntermediates VertexFactoryIntermediates()
{
	FVertexFactoryIntermediates VFIntermediates;
	RenderPatch current_patch = patches[gl_InstanceIndex]; 
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
	VFIntermediates.MipmapLevel = min(NumHeightfieldTextureMipmap, current_patch.lod);
	ivec2 id = ivec2(HeightTexture_UV * vec2(HeightfieldWidth, HeightfieldHeight));
	CalcNormalTangent(id, VFIntermediates.MipmapLevel, VFIntermediates.Normal, VFIntermediates.Tangent);
	VFIntermediates.ModelToWorld = LocalToWorld * ModelToLocal;
	return VFIntermediates;
}
dvec3 VertexFactoryGetWorldPosition(FVertexFactoryIntermediates VFIntermediates)
{
	float height = textureLod(HeightfieldTexture, VFIntermediates.Heightfield_UV, VFIntermediates.MipmapLevel).r;
	VFIntermediates.pos.z += height;

	return dvec3(VFIntermediates.ModelToWorld * dvec4(VFIntermediates.pos, 1));
}

vec3 VertexFactoryGetWorldNormal(FVertexFactoryIntermediates VFIntermediates)
{
	return mat3(transpose(inverse(mat3(VFIntermediates.ModelToWorld)))) * VFIntermediates.Normal;
}

vec4 VertexFactoryGetWorldTangent(FVertexFactoryIntermediates VFIntermediates)
{
	return mat4(VFIntermediates.ModelToWorld) * VFIntermediates.Tangent;
}
vec2 VertexFactoryGetTexCoord(FVertexFactoryIntermediates VFIntermediates)
{
	return TEXCOORD_0;
}

vec4 VertexFactoryGetColor(FVertexFactoryIntermediates VFIntermediates)
{
	RenderPatch current_patch = patches[gl_InstanceIndex]; 
	return vec4(current_patch.lod / 5.0);
//	return vec4(VFIntermediates.MinMax/vec2(10), 0, 1);
}