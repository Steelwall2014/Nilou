#version 460 core
#include "../include/RenderPatch.glsl"

layout(location = 0) in vec2 POSITION;
layout(location = 3) in vec2 TEXCOORD_0;
layout(std430, binding=4) readonly buffer Patch_Buffer{
    RenderPatch patches[];
};
out vec2 UV;
out vec3 frag_position;
out float LOD;
//uniform mat4 model;
uniform mat4 VP;
uniform sampler2D DisplaceMap;
uniform sampler2D PerlinNoise;
uniform float OceanTextureMeterSize;
uniform float PatchOriginalGridMeterSize;
uniform uint PatchGridSideNum;
uniform vec3 cameraPos;

uniform vec2 WindDirection;
uniform float Time;

#define BLEND_START  10    // m
#define BLEND_END    300  // m

const vec3 perlinFrequency = vec3(1.12, 0.59, 0.23);
const vec3 perlinAmplitude = vec3(0.35, 0.42, 0.57);
const vec3 perlinGradient  = vec3(0.014, 0.016, 0.022);

#include "../include/FixLODSeam.glsl"

void main(){
	RenderPatch current_patch = patches[gl_InstanceID]; 
	float scale = pow(2, current_patch.lod);
	vec3 offset = vec3(current_patch.offset_x, current_patch.offset_y, 0);

	vec3 pos = vec3(POSITION , 0);
	pos *= scale;
	pos += offset;
	float dist = length((cameraPos - pos).xy);
	float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);

	FixLODSeam(pos, scale, current_patch);

	UV = pos.xy / OceanTextureMeterSize;
	vec3 displacement = texture2D(DisplaceMap, UV).rgb;
	
	float p0 = texture(PerlinNoise, UV * perlinFrequency.x - WindDirection * Time * 0.06f).a;
	float p1 = texture(PerlinNoise, UV * perlinFrequency.y - WindDirection * Time * 0.06f).a;
	float p2 = texture(PerlinNoise, UV * perlinFrequency.z - WindDirection * Time * 0.06f).a;

	float perl = dot(vec3(p0, p1, p2), perlinAmplitude);

	displacement = mix(vec3(0.0, 0, perl), displacement, factor);
	pos += displacement;
	
	gl_Position = VP * vec4(pos, 1.0f);
	frag_position = pos;
	LOD = current_patch.lod;
}
