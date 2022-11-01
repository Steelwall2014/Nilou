#version 430 core
#include "../include/RenderPatch.glsl"

layout(location = 0) in vec2 POSITION;
layout(location = 3) in vec2 TEXCOORD_0;
layout(std430, binding=4) readonly buffer Patch_Buffer{
    RenderPatch patches[];
};
out vec2 UV;
out vec3 frag_position;
out float LOD;
out float OnEdge;

out vec2 Texture_UV;
uniform float TextureMapMeterSize;

uniform mat4 VP;
uniform float HeightMapMeterSize;
uniform float PatchOriginalGridMeterSize;
uniform uint PatchGridSideNum;

uniform sampler2D HeightMap;

#include "../include/FixLODSeam.glsl"

void main(){
	RenderPatch current_patch = patches[gl_InstanceID]; 
	float scale = pow(2, current_patch.lod);
	vec3 offset = vec3(current_patch.offset_x, current_patch.offset_y, 0);

	vec3 pos = vec3(POSITION , 0);
	pos *= scale;
	pos += offset;

	OnEdge = float(FixLODSeam(pos, scale, current_patch));

	UV = pos.xy / HeightMapMeterSize;
	Texture_UV = pos.xy / TextureMapMeterSize;

	float height = texture(HeightMap, UV).r;
	pos.z += height;
	gl_Position = VP * vec4(pos, 1.0f);
	frag_position = pos;
	LOD = current_patch.lod;
}
