#version 460 core
#include "../include/unit_definitions.glsl"
#include "../atmosphere/atmosphere_definitions.glsl"
#include "../atmosphere/atmosphere_functions.glsl"

in vec2 UV;
in vec3 frag_position;
in vec3 VertexNormal;
in float LOD;

layout (location = 0) out vec4 FragColor;

uniform sampler2D PerlinNoise;
uniform sampler2D DisplaceMap;
uniform sampler2D NormalMap;
uniform sampler2D FoamMap;
uniform samplerCube skybox;
uniform sampler2D skymap;
uniform sampler2D TransmittanceLUT;
uniform sampler3D SingleScatteringRayleighLUT;
uniform sampler3D SingleScatteringMieLUT;

uniform sampler3D OceanScatteringLUT;
layout (rgba32f, binding = 0) uniform image2D SlopeVariance;

uniform vec3 cameraPos;
uniform float MaxLOD;
uniform float A;
uniform float OceanTextureMeterSize;

struct Light
{
        int        lightType;
        vec3       lightPosition;
        vec4       lightColor;
        vec3       lightDirection;
        float      lightIntensity;
        int        lightDistAttenCurveType;
        float      lightDistAttenCurveParams[5];
        int        lightAngleAttenCurveType;
        float      lightAngleAttenCurveParams[5];
        bool       lightCastShadow;
        int        lightShadowMapLayerIndex;
        mat4       lightVP;
};
uniform Light lights[5];
float lerp(float from, float to, float t)
{
    return from + (to-from) * t;
}

/****************/
#define ONE_OVER_4PI	0.0795774715459476
#define BLEND_START  10    // m
#define BLEND_END    300  // m
float SubSurfaceSunFallOff = 5;
float SubSurfaceBase = 0.33;
float SubSurfaceSun = 1.13;
vec3 SubSurfaceColour = vec3(18,65,76) / 255;
float F0 = 0.02;
const vec3 perlinFrequency	= vec3(1.12, 0.59, 0.23);
const vec3 perlinGradient	= vec3(0.014, 0.016, 0.022);
/****************/
uniform vec2 WindDirection;
uniform float Time;

float P(vec2 zetaH, float sigmaXsq, float sigmaYsq)
{
    float zetax = zetaH.x;
    float zetay = zetaH.y;
    float p = exp(-0.5 * (zetax * zetax / sigmaXsq + zetay * zetay / sigmaYsq)) / (2.0 * PI * sqrt(sigmaXsq * sigmaYsq));
    return p;
}
float erfc(float x) {
	return 2.0 * exp(-x * x) / (2.319 * x + sqrt(4.0 + 1.52 * x * x));
}
float Lambda(float cosTheta, float sigmaSq) {
	float v = cosTheta / sqrt((1.0 - cosTheta * cosTheta) * (2.0 * sigmaSq));
    return max(0.0, (exp(-v * v) - v * sqrt(PI) * erfc(v)) / (2.0 * v * sqrt(PI)));
}
float meanFresnel(float cosThetaV, float sigmaV) {
	return pow(1.0 - cosThetaV, 5.0 * exp(-2.69 * sigmaV)) / (1.0 + 22.7 * pow(sigmaV, 1.5));
}

void main()
{

	vec3 L = normalize(-lights[0].lightDirection);
	Length r = ATMOSPHERE.bottom_radius;
	Number mu = L.z;
    vec3 transmittance = GetTransmittanceToTopAtmosphereBoundary(
          ATMOSPHERE, TransmittanceLUT, r, mu);
	vec3 sunColor = transmittance * ATMOSPHERE.solar_irradiance;

	float dist = length((cameraPos - frag_position).xy);
	float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
	factor = clamp(factor * factor, 0.0, 1.0);

	vec2 p0 = texture(PerlinNoise, UV * perlinFrequency.x - WindDirection * Time * 0.06f).rg;
	vec2 p1 = texture(PerlinNoise, UV * perlinFrequency.y - WindDirection * Time * 0.06f).rg;
	vec2 p2 = texture(PerlinNoise, UV * perlinFrequency.z - WindDirection * Time * 0.06f).rg;

	vec2 perl = (p0 * perlinGradient.x + p1 * perlinGradient.y + p2 * perlinGradient.z);
	vec3 grad = texture(NormalMap, UV).xyz;
	grad.xy = mix(perl, grad.xy, factor);

	vec3 N = normalize(grad.xyz);
	vec3 V = normalize(cameraPos - frag_position);
	vec3 R = reflect(-V, N);
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(N, V), 5.0);
	vec3 H = normalize(L + V);

	// hack���ڷ������߹���ˮƽʱ��ֹ��������
	if (R.z < 0.08)
		R = normalize(vec3(R.xy, 0.08));

        
    /*****************������չ�*********************/
    // ��Ԥ��������ͼ����
	vec2 skymap_uv = GetUVFromViewVector(R);
	vec3 refl = texture(skymap, skymap_uv).rgb * 10;
        

	float spec;

    /*****************Bruneton̫���߹�*********************/
    float zL = dot(L, N);
    float zV = dot(V, N);
    float zH = dot(H, N);
    float zH2 = zH * zH;

    vec3 Ty = normalize(vec3(0, N.z, -N.y));
    vec3 Tx = cross(Ty, N);

    vec2 zetaH = -vec2(dot(H, Tx), dot(H, Ty)) / dot(H, N);

    float cosThetaV = dot(V, N);
    float cosThetaL = dot(L, N);

    float phiV = atan(dot(V, Ty), dot(V, Tx));
    float phiL = atan(dot(L, Ty), dot(L, Tx));
            
    float sigmaFactor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
    float sigmaXsq = lerp(0.015, 0.0015, factor);
    float sigmaYsq = lerp(0.015, 0.0015, factor);
    vec2 sigmaSq = vec2(sigmaXsq, sigmaYsq);

    float sigmaV = sqrt(sigmaXsq * cos(phiV) * cos(phiV) + sigmaYsq * sin(phiV) * sin(phiV));

    float zetaHx = dot(H, Tx) / dot(H, N);
    float zetaHy = dot(H, Ty) / dot(H, N);
    float p = P(vec2(zetaHx, zetaHy), sigmaXsq, sigmaYsq);

    float fresnel = F0 + (1-F0) * pow(1.0 - dot(V, H), 5.0);

    float tanV = atan(dot(V, Ty), dot(V, Tx));
    float cosV2 = 1.0 / (1.0 + tanV * tanV);
    float sigmaV2 = sigmaSq.x * cosV2 + sigmaSq.y * (1.0 - cosV2);

    float tanL = atan(dot(L, Ty), dot(L, Tx));
    float cosL2 = 1.0 / (1.0 + tanL * tanL);
    float sigmaL2 = sigmaSq.x * cosL2 + sigmaSq.y * (1.0 - cosL2);

    float denominator = (1.0 + Lambda(zL, sigmaL2) + Lambda(zV, sigmaV2)) * zV * zH2 * zH2 * 4.0;
    // hack�������ɫģ����grazing angle��ʱ���п��ܻ���ֳ������
    if (denominator < 1e-6)
        spec = p * fresnel;
    else
        spec = p * fresnel / denominator;
    const float sunIntensityScale = 5;   // �ֶ������Ĳ���
    spec *= sunIntensityScale;

	/*****************�α���ɢ��*********************/
    float sssIntensity = 1;
    float towardsSun = pow(max(0., dot(L, -V)), SubSurfaceSunFallOff);
	vec3 subsurface = (SubSurfaceBase + SubSurfaceSun * towardsSun) * SubSurfaceColour.rgb * sunColor;

    vec3 in_scatter = GetSkyRadianceToPoint(ATMOSPHERE,
        TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
        vec3(0, 0, cameraPos/km) - earth_center, (frag_position - cameraPos)/km - earth_center, 0, L, transmittance);

	vec3 color = vec3(0);
    color += subsurface * (1-F);
    color += refl * F;
    color += sunColor * spec;

    float in_scatter_scale = 2.5;   // �ֶ������Ĳ���
    FragColor = vec4(hdr(color * transmittance + in_scatter*in_scatter_scale, 0.4), 1);

}