/**
* The ocean surface rendering combines https://darthasylum.blog.hu/2018/09/17/ocean_rendering,
"Real-time Realistic Ocean Lighting using Seamless Transitions from Geometry to BRDF" and 
Crest Ocean System
*/
//#version 460
#ifndef OCEAN_SURFACE_H
#define OCEAN_SURFACE_H

#include "../include/functions.glsl"
#include "../include/PBRFunctions.glsl"
#include "../include/Maths.glsl"
#include "../include/LightShaderParameters.glsl"
#include "../include/ViewShaderParameters.glsl"
#include "../SkyAtmosphere/atmosphere_functions.glsl"
#include "ShadingParams.glsl"
#include "SkyAtmosphereLUTs.glsl"

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
	//return (exp(-v * v)) / (2.0 * v * sqrt(M_PI)); // approximate, faster formula
}

float meanFresnel(float cosThetaV, float sigmaV) {
	return pow(1.0 - cosThetaV, 5.0 * exp(-2.69 * sigmaV)) / (1.0 + 22.7 * pow(sigmaV, 1.5));
}

// V, N in world space
float meanFresnel(vec3 V, vec3 N, vec2 sigmaSq) {
    vec2 v = V.xy; // view direction in wind space
    vec2 t = v * v / (1.0 - V.z * V.z); // cos^2 and sin^2 of view direction
    float sigmaV2 = dot(t, sigmaSq); // slope variance in view direction
    return meanFresnel(dot(V, N), sqrt(sigmaV2));
}
vec3 ApplyOceanSubsurface(FLightShaderParameters light, ShadingParams params)
{
    vec3 FoamColor = vec3(1);

    // baseColor is taken as SubSurfaceColour in ocean surface rendering
	vec3 SubSurfaceColour = params.baseColor;
    
    // emissive is taken as SubSurfaceBase, SubSurfaceSun and SubSurfaceSunFallOff in ocean surface rendering
//    float SubSurfaceBase = params.emissive.x;
//    float SubSurfaceSun = params.emissive.y;
//    float SubSurfaceSunFallOff = params.emissive.z;
    float SubSurfaceBase = 0.33;
    float SubSurfaceSun = 1.13;
    float SubSurfaceSunFallOff = 5;
    
    // metallic and roughness are taken as sigmaXsq and sigmaYsq in ocean surface rendering
    float sigmaXsq = params.metallic;
    float sigmaYsq = params.roughness; 

    
    vec3 N = params.N;
    vec3 L = params.L;
    vec3 H = params.H;
    vec3 V = params.V;
    vec3 R = reflect(-V, N);
	if (R.z < 0.08)
		R = normalize(vec3(R.xy, 0.08));
    vec3 refl = HDR(GetSkyColor(
            ATMOSPHERE, TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT, 
            -earth_center.z, R, 0, L, ATMOSPHERE.sun_angular_radius), 10);
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
            
    vec2 sigmaSq = vec2(sigmaXsq, sigmaYsq);

    float sigmaV = sqrt(sigmaXsq * cos(phiV) * cos(phiV) + sigmaYsq * sin(phiV) * sin(phiV));

    float zetaHx = dot(H, Tx) / dot(H, N);
    float zetaHy = dot(H, Ty) / dot(H, N);
    float p = P(vec2(zetaHx, zetaHy), sigmaXsq, sigmaYsq);

    float F0 = 0.02;
    float fresnel = F0 + (1-F0) * meanFresnel(V, N, sigmaSq);//pow(1.0 - dot(V, H), 5.0);

    float tanV = atan(dot(V, Ty), dot(V, Tx));
    float cosV2 = 1.0 / (1.0 + tanV * tanV);
    float sigmaV2 = sigmaSq.x * cosV2 + sigmaSq.y * (1.0 - cosV2);

    float tanL = atan(dot(L, Ty), dot(L, Tx));
    float cosL2 = 1.0 / (1.0 + tanL * tanL);
    float sigmaL2 = sigmaSq.x * cosL2 + sigmaSq.y * (1.0 - cosL2);

    float denominator = (1.0 + Lambda(zL, sigmaL2) + Lambda(zV, sigmaV2)) * zV * zH2 * zH2 * 4.0;
    float spec;
    if (denominator < 1e-6)
        spec = p * fresnel;
    else
        spec = p * fresnel / denominator;
//    denominator = max(denominator, 1e-6);

    float vz = abs(V.z);
    float towardsSun = pow(max(0., dot(L, -V)), SubSurfaceSunFallOff);
    vec3 subsurface = (SubSurfaceBase + SubSurfaceSun * towardsSun) * SubSurfaceColour.rgb * light.lightIntensity / 10;
    vec3 color = vec3(0);
    color += subsurface * (1-fresnel);
    color += refl * fresnel;
    color += light.lightIntensity * spec * 5 / 10;
    return 0.4 * color;
}
#endif