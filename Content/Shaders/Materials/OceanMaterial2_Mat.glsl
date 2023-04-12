#version 460
#include "../include/functions.glsl"
#include "../include/PBRFunctions.glsl"
#include "../include/BasePassCommon.glsl" 
#include "../include/ViewShaderParameters.glsl" 
#include "../FastFourierTransformOcean/OceanFastFourierTransformParameters.glsl" 
uniform sampler2D PerlinNoise;
uniform sampler2D DisplaceTexture;
uniform sampler2D NormalTexture;        
#define ONE_OVER_4PI	0.0795774715459476
#define BLEND_START  10    // m
#define BLEND_END    300  // m

vec4 MaterialGetBaseColor(VS_Out vs_out)            
{        
	return vec4(LinearToGamma(vec3(18,65,76) / 255), 1);
}            
vec3 MaterialGetEmissive(VS_Out vs_out)            
{              
	return vec3(0);            
}            
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)            
{                
	float dist = length(vs_out.RelativeWorldPosition.xy);
	float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
//	factor = clamp(factor * factor, 0.0, 1.0);
	dvec3 WorldPosition = vs_out.RelativeWorldPosition + CameraPosition;
	vec2 UV = vec2(WorldPosition.xy / dvec2(DisplacementTextureSize));
	
	const vec3 perlinFrequency	= vec3(1.12, 0.59, 0.23);
	const vec3 perlinGradient	= vec3(0.014, 0.016, 0.022);
	vec2 p0 = texture(PerlinNoise, UV * perlinFrequency.x - WindDirection * Time * 0.06f).rg;
	vec2 p1 = texture(PerlinNoise, UV * perlinFrequency.y - WindDirection * Time * 0.06f).rg;
	vec2 p2 = texture(PerlinNoise, UV * perlinFrequency.z - WindDirection * Time * 0.06f).rg;
	
	vec2 perl = (p0 * perlinGradient.x + p1 * perlinGradient.y + p2 * perlinGradient.z);
	vec3 grad = texture(NormalTexture, UV).xyz;
	grad.xy = mix(perl, grad.xy, factor);

	return normalize(grad.xyz);            
}           
float MaterialGetMetallic(VS_Out vs_out)            
{               
	return 0.0;            
}          
float MaterialGetRoughness(VS_Out vs_out)            
{          
	return 0.4;            
}               
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)            
{                
	float dist = length(vs_out.RelativeWorldPosition.xy);
	float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
	dvec3 WorldPosition = vs_out.RelativeWorldPosition + CameraPosition;
	vec2 UV = vec2(WorldPosition.xy / dvec2(DisplacementTextureSize));

	vec3 displacement = texture(DisplaceTexture, UV).rgb;
	
//	const vec3 perlinFrequency = vec3(1.12, 0.59, 0.23);
//	const vec3 perlinAmplitude = vec3(0.35, 0.42, 0.57);
//	float p0 = texture(PerlinNoise, UV * perlinFrequency.x - WindDirection * Time * 0.06f).a;
//	float p1 = texture(PerlinNoise, UV * perlinFrequency.y - WindDirection * Time * 0.06f).a;
//	float p2 = texture(PerlinNoise, UV * perlinFrequency.z - WindDirection * Time * 0.06f).a;
//
//	float perl = dot(vec3(p0, p1, p2), perlinAmplitude);
//	displacement = mix(vec3(0.0, 0, perl), displacement, factor);

	return displacement;            
}        

        