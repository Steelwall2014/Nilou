#version 460 core

#include "include/MinMaxLODNum.glsl"
#include "include/RenderPatch.glsl"
#include "include/WorldLODParam.glsl"

layout(location = 0) in vec3 inputPosition;
layout(std430, binding=1) readonly buffer Patch_Buffer{
    RenderPatch patches[];
};
layout(std430, binding=2) readonly buffer LODParams_Buffer{
    WorldLODParam LODParams[];
};
uniform sampler2D MinMaxMap[MAX_LOD_NUM+3];

uniform mat4 VP;

out vec3 Culled;

#include "include/FrustumCull.glsl"


void main(){
	RenderPatch current_patch = patches[gl_InstanceID]; 
    uint LOD = current_patch.lod;
	vec3 offset = vec3(current_patch.offset_x, current_patch.offset_y, 0);

    float patch_meter_size = LODParams[LOD].NodeMeterSize / 8.0;
    ivec2 patchLoc = ivec2(offset / patch_meter_size);
    vec2 uv = (vec2(patchLoc) + vec2(0.5)) / (8.0 * float(LODParams[LOD].NodeSideNum));
    vec2 MinMax = texture2D(MinMaxMap[LOD], uv).rg;
    offset.z = MinMax.r;
    vec3 scale = vec3(patch_meter_size, patch_meter_size, MinMax.g - MinMax.r);
    vec3 pos = (vec3(0.5, 0.5, 0.5) + inputPosition);
    pos *= scale;
    pos += offset;

    vec3 box[8];
    vec2 upper_left = vec2(current_patch.offset_x, current_patch.offset_y);
    vec2 lower_right = upper_left + vec2(patch_meter_size);
    CreateBox(upper_left, lower_right, MinMax, box);
    if (FrustumCull(box))
    {
        Culled = vec3(0, 0, 1);
    }
    else
    {
        Culled = vec3(1, 0, 0);
    }
	gl_Position = VP * vec4(pos, 1.0f);
}
