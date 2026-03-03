implementing LightingPassPixelShader;

/**
 * The ocean surface rendering combines https://darthasylum.blog.hu/2018/09/17/ocean_rendering,
 * "Real-time Realistic Ocean Lighting using Seamless Transitions from Geometry to BRDF" and
 * Crest Ocean System
 */

// float P(float2 zetaH, float sigmaXsq, float sigmaYsq)
// {
//     float zetax = zetaH.x;
//     float zetay = zetaH.y;
//     float p     = exp(-0.5f * (zetax * zetax / sigmaXsq + zetay * zetay / sigmaYsq)) / (2.0f * PI * sqrt(sigmaXsq * sigmaYsq));
//     return p;
// }

// float erfc(float x)
// {
//     return 2.0f * exp(-x * x) / (2.319f * x + sqrt(4.0f + 1.52f * x * x));
// }

// float Lambda(float cosTheta, float sigmaSq)
// {
//     float v = cosTheta / sqrt((1.0f - cosTheta * cosTheta) * (2.0f * sigmaSq));
//     return max(0.0f, (exp(-v * v) - v * sqrt(PI) * erfc(v)) / (2.0f * v * sqrt(PI)));
// }

// float meanFresnel(float cosThetaV, float sigmaV)
// {
//     return pow(1.0f - cosThetaV, 5.0f * exp(-2.69f * sigmaV)) / (1.0f + 22.7f * pow(sigmaV, 1.5f));
// }

// // V, N in world space
// float meanFresnel(float3 V, float3 N, float2 sigmaSq)
// {
//     float2 v = V.xy;
//     float2 t = v * v / (1.0f - V.z * V.z);
//     float  sigmaV2 = dot(t, sigmaSq);
//     return meanFresnel(dot(V, N), sqrt(sigmaV2));
// }

// float3 ApplyOceanSubsurface(FLightShaderParameters light, ShadingParams params)
// {
//     float3 FoamColor = float3(1.0f);

//     float3 SubSurfaceColour = params.baseColor;

//     float SubSurfaceBase      = 0.33f;
//     float SubSurfaceSun       = 1.13f;
//     float SubSurfaceSunFallOff = 5.0f;

//     float sigmaXsq = params.metallic;
//     float sigmaYsq = params.roughness;

//     float3 N = params.N;
//     float3 L = params.L;
//     float3 H = params.H;
//     float3 V = params.V;
//     float3 R = reflect(-V, N);
//     if (R.z < 0.08f)
//         R = normalize(float3(R.xy, 0.08f));
//     float3 refl = HDR(GetSkyColor(
//         ATMOSPHERE, TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT,
//         -earth_center.z, R, 0.0f, L, ATMOSPHERE.sun_angular_radius), 10.0f);

//     float zL  = dot(L, N);
//     float zV  = dot(V, N);
//     float zH  = dot(H, N);
//     float zH2 = zH * zH;

//     float3 Ty = normalize(float3(0.0f, N.z, -N.y));
//     float3 Tx = cross(Ty, N);

//     float2 zetaH = -float2(dot(H, Tx), dot(H, Ty)) / dot(H, N);

//     float cosThetaV = dot(V, N);
//     float cosThetaL = dot(L, N);

//     float phiV = atan2(dot(V, Ty), dot(V, Tx));
//     float phiL = atan2(dot(L, Ty), dot(L, Tx));

//     float2 sigmaSq = float2(sigmaXsq, sigmaYsq);

//     float sigmaV = sqrt(sigmaXsq * cos(phiV) * cos(phiV) + sigmaYsq * sin(phiV) * sin(phiV));

//     float zetaHx = dot(H, Tx) / dot(H, N);
//     float zetaHy = dot(H, Ty) / dot(H, N);
//     float p      = P(float2(zetaHx, zetaHy), sigmaXsq, sigmaYsq);

//     float F0      = 0.02f;
//     float fresnel = F0 + (1.0f - F0) * meanFresnel(V, N, sigmaSq);

//     float tanV   = atan2(dot(V, Ty), dot(V, Tx));
//     float cosV2  = 1.0f / (1.0f + tanV * tanV);
//     float sigmaV2 = sigmaSq.x * cosV2 + sigmaSq.y * (1.0f - cosV2);

//     float tanL    = atan2(dot(L, Ty), dot(L, Tx));
//     float cosL2   = 1.0f / (1.0f + tanL * tanL);
//     float sigmaL2 = sigmaSq.x * cosL2 + sigmaSq.y * (1.0f - cosL2);

//     float denominator = (1.0f + Lambda(zL, sigmaL2) + Lambda(zV, sigmaV2)) * zV * zH2 * zH2 * 4.0f;
//     float spec;
//     if (denominator < 1e-6f)
//         spec = p * fresnel;
//     else
//         spec = p * fresnel / denominator;

//     float  vz         = abs(V.z);
//     float  towardsSun = pow(max(0.0f, dot(L, -V)), SubSurfaceSunFallOff);
//     float3 subsurface = (SubSurfaceBase + SubSurfaceSun * towardsSun) * SubSurfaceColour.rgb * light.lightIntensity / 10.0f;
//     float3 color      = float3(0.0f);
//     color += subsurface * (1.0f - fresnel);
//     color += refl * fresnel;
//     color += light.lightIntensity * spec * 5.0f / 10.0f;
//     return 0.4f * color;
// }
