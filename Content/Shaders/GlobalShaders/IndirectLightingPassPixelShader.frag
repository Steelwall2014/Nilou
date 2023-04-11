#version 460

layout (location = 0) out vec4 FragColor;

uniform sampler2D BaseColor;
uniform sampler2D RelativeWorldSpacePosition;
uniform sampler2D WorldSpaceNormal;
uniform sampler2D MetallicRoughness;
uniform sampler2D Emissive;
uniform usampler2D ShadingModel;

layout (std430, binding = 1) buffer IrradianceTexturesBlock {

    uvec2 IrradianceTextureHandles[];

};

layout (std430, binding = 2) buffer PrefiltededTexturesBlock {

    uvec2 PrefiltededTextureHandles[];

};

void main()
{
}