#version 460 core
#include "../include/unit_definitions.glsl"
#include "../atmosphere/atmosphere_definitions.glsl"
#include "../atmosphere/atmosphere_functions.glsl"

#include "../waterbody/waterbody_constants.glsl"
#include "../waterbody/waterbody_definitions.glsl"
#include "../waterbody/waterbody_functions.glsl"
in vec2 UV;
in vec3 frag_position;
in vec3 VertexNormal;
in float LOD;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 SonarHUDColor;

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
//uniform vec2 sigmaSq;

#include "../include/Light.glsl"
#include "../include/functions.glsl"
#include "../include/PBRFunctions.glsl"
#include "../atmosphere/atmosphere_skymap_functions.glsl"

/****************/
#define ONE_OVER_4PI	0.0795774715459476
#define BLEND_START  10    // m
#define BLEND_END    300  // m
vec3 FoamColor = vec3(1);
float SubSurfaceSunFallOff = 5;
float SubSurfaceBase = 0.33;
float SubSurfaceSun = 1.13;
vec3 SubSurfaceColour = vec3(18,65,76) / 255;   // TODO ��ˮ��ɢ������
//vec3 SeaColor = vec3(0.050980397, 0.1058824, 0.2);
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
	//return (exp(-v * v)) / (2.0 * v * sqrt(M_PI)); // approximate, faster formula
}
float meanFresnel(float cosThetaV, float sigmaV) {
	return pow(1.0 - cosThetaV, 5.0 * exp(-2.69 * sigmaV)) / (1.0 + 22.7 * pow(sigmaV, 1.5));
}

void main()
{
//    vec2 dfdx = dFdx(frag_position.xy);
//    vec2 dfdy = dFdy(frag_position.xy);
    if (mod(frag_position.x, OceanTextureMeterSize) < 0.2 || mod(frag_position.x, OceanTextureMeterSize) > OceanTextureMeterSize-0.2 || 
        mod(frag_position.y, OceanTextureMeterSize) < 0.2 || mod(frag_position.y, OceanTextureMeterSize) > OceanTextureMeterSize-0.2)
        SonarHUDColor = vec4(0.7, 0.7, 0.7, 1);
    else
        SonarHUDColor = vec4(0.1, 0.1, 0.1, 1);
//    if (dfdx.x*dfdy.y - dfdy.x*dfdx.y > 0.4*0.4)
//        SonarHUDColor = vec4(0.4, 0.4, 0.4, 1);
//    vec3 color;
//    if (-0.5 < LOD && LOD < 0.5)
//        color = vec3(0, 1, 0);
//    else if (0.5 < LOD && LOD < 1.5)
//        color = vec3(0, 0, 1);
//    else if (1.5 < LOD && LOD < 2.5)
//        color = vec3(1, 0, 0);
//    else if (2.5 < LOD && LOD < 3.5)
//        color = vec3(1, 1, 0);
//    else if (3.5 < LOD && LOD < 4.5)
//        color = vec3(0, 1, 1);
//    else if (4.5 < LOD && LOD < 5.5)
//        color = vec3(1, 0, 1);
//    else if (5.5 < LOD && LOD < 6.5)
//        color = vec3(1, 1, 1);
//    FragColor = vec4(color, 1);
//    return;

	vec3 L = normalize(-lights[0].lightDirection);
	Length r = ATMOSPHERE.bottom_radius;
	Number mu = L.z;
    vec3 transmittance = GetTransmittanceToTopAtmosphereBoundary(
          ATMOSPHERE, TransmittanceLUT, r, mu);
//	vec3 oceanColor = SeaColor;
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
//	vec3 N = normalize(vec3(0, 1, 1));
	vec3 V = normalize(cameraPos - frag_position);
	vec3 R = reflect(-V, N);
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(N, V), 5.0);
        

    if (cameraPos.z < texture2D(DisplaceMap, UV).z)
    {
        vec3 refracted_L = -refract(-L, vec3(0, 0, 1), 1 / 1.333);
        vec3 refract_V = refract(-V, -N, 1.333);
        vec3 radiance = vec3(0);
	    if (dot(-V, N) < cos(radians(48.0)))    // ������ȫ����
        {
            vec3 reflect_dir = reflect(-V, -N);
		    float r = WATERBODY.water_depth-10;  // �������ȡ��water_depth�Ļ��еĵط����ܻ����һƬ��ɫ
		    float mu = acos(reflect_dir.z);
		    float mu_s = acos(refracted_L.z);
		    float nu = acos(dot(refracted_L, reflect_dir));
            vec3 scattering = GetScattering(WATERBODY, OceanScatteringLUT, r, mu, mu_s, nu, false);
            radiance = scattering/2;
//                reflect_color = hdr(scattering, 1);
//                FragColor = vec4(reflect_color, 1);
        }
        else
        {
            vec3 refract_radiance = GetSkyColor(
                ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
                -earth_center.z, refract_V, 0, L, ATMOSPHERE.sun_angular_radius);
            radiance = refract_radiance;
//                refract_color = refract_color * (1-fresnel) * 10;
//                FragColor = vec4(refract_color, 1);
        }
//        radiance *= GetTransmittance(WATERBODY, length(frag_position-cameraPos));
//	    float r = WATERBODY.water_depth + cameraPos.z;
//	    float mu = acos(clamp(V.z, -1, 1));
//	    float mu_s = acos(clamp(refracted_L.z, -1, 1));
//	    float nu = acos(clamp(dot(refracted_L, V), -1, 1));
//        vec3 in_scattering = GetScattering(WATERBODY, OceanScatteringLUT, r, mu, mu_s, nu, false);
//        radiance += in_scattering * 10;
        FragColor = vec4(hdr(radiance, 10), 1);
//        FragColor = vec4(radiance, 1);
//            FragColor = vec4(refract_color * (1-fresnel), 1);
    }
    else
    {
        if (gl_FrontFacing)
        {
		    // hack���ڷ������߹���ˮƽʱ��ֹ��������
	        if (R.z < 0.08)
		        R = normalize(vec3(R.xy, 0.08));
            // ��Ԥ��������ͼ�������Ƽ���
	        vec2 skymap_uv = GetUVFromViewVector(R);
	        vec3 refl = texture(skymap, skymap_uv).rgb * 10;
        
            // ֱ�Ӽ������ɢ��
            /*
            vec3 refl = hdr(GetSkyColor(
                ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
                -earth_center.z, R, 0, L, ATMOSPHERE.sun_angular_radius), 10);
            */

	        float turbulence = max(1.6 - texture2D(FoamMap, UV).r, 0.0);
	        float color_mod = 1.0 + 3.0 * smoothstep(1.2, 1.6, turbulence);

	        color_mod = mix(1.0, color_mod, factor);
	        vec3 H = L + V;

	        float spec;

            /*****************Ward BRDF*********************/
    //        	const float rho = 0.3;
    //        	const float ax = 0.2;
    //        	const float ay = 0.1;
    //        
    //        	H = L + V;
    //        	vec3 x = cross(L, N);
    //        	vec3 y = cross(x, N);
    //        
    //        	float mult = (ONE_OVER_4PI * rho / (ax * ay * sqrt(max(1e-5, dot(L, N) * dot(V, N)))));
    //        	float hdotx = dot(H, x) / ax;
    //        	float hdoty = dot(H, y) / ay;
    //        	float hdotn = dot(H, N);
    //        
    //        	spec = mult * exp(-((hdotx * hdotx) + (hdoty * hdoty)) / (hdotn * hdotn));


            /*****************Cooley-Tukey BRDF*********************/
            //	float roughness = 0.1;
            //    // ����߹�
            //	H = normalize(H);
            //    float NdotL = max(dot(N, L), 0.0); 
            //    float NdotV = max(dot(N, V), 0.0); 
            //    float fresnel = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
            //    float G = GeometrySmith(NdotL, NdotV, roughness);
            //    float NDF = NDF_GGXTR(N, H, roughness);
            //    float nominator = NDF * G * fresnel;
            //    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
            //    spec = nominator / denominator;


            /*****************Bruneton*********************/
            H = normalize(H);
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

	        sunColor *= clamp((L.z+sin(ATMOSPHERE.sun_angular_radius)) / sin(ATMOSPHERE.sun_angular_radius*2), 0, 1);
	        // �α���ɢ��
            float vz = abs(V.z);
            float sssIntensity = 1;//frag_position.z / A / 2;     // �ò�����ģ��α���ɢ���ǿ�ȣ�����һ�ֽ��Ƶ�ģ�⣬����Ч������
            float towardsSun = pow(max(0., dot(L, -V)), SubSurfaceSunFallOff);
	        vec3 subsurface = (SubSurfaceBase + SubSurfaceSun * towardsSun) * SubSurfaceColour.rgb * sunColor;
    //	    subsurface *= clamp((1.0 - vz * vz) * sssIntensity, 0.6f, 1.0f);
    //        oceanColor += subsurface;

            vec3 transmittance;
            vec3 in_scatter = GetSkyRadianceToPoint(ATMOSPHERE,
                TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
                vec3(0, 0, cameraPos/km) - earth_center, (frag_position - cameraPos)/km - earth_center, 0, L, transmittance);
    //	    vec3 color = mix(subsurface, refl/* * color_mod*/, F) + sunColor * spec;// + subsurface;

	        vec3 color = vec3(0);
            color += subsurface * (1-F);
            color += refl * F;
            color += sunColor * spec;

            float in_scatter_scale = 2.5;   // �ֶ������Ĳ���
            FragColor = vec4(hdr(color * transmittance + in_scatter*in_scatter_scale, 0.4), 1);
    //        FragColor = vec4(hdr(color, 0.4), 1);
    //        FragColor = vec4(refl, 1);
        }
        else
        {
            discard;
        }
    }
}

//void main(){
////    vec3 _LightColor0 = vec3(lights[0].lightColor);
////    vec3 _Specular = vec3(0.4962264, 0.4943574, 0.4943574);
////    float _Gloss = 256;
////    float _FresnelScale = 0.04;
//    //FragColor = vec4(UV, 0, 1);
//    vec3 L;
//    if (lights[0].lightType == 2)   // Directional
//        L = normalize(-lights[0].lightDirection);
//    else 
//        L = normalize(lights[0].lightPosition - frag_position);
//    vec3 N = normalize(texture(NormalMap, UV)).xyz;
//    float mipmapLevel = textureQueryLod(NormalMap, UV).x;
////    vec3 N = vec3(0, 0, 1);
//    float camera_frag_dist = distance(cameraPos, frag_position);
//    vec3 V = normalize(cameraPos - frag_position);
//    vec3 H = normalize(L + V);
//    float NdotL = max(dot(N, L), 0.0); 
//    float NdotV = max(dot(N, V), 0.0); 
//
//    float foam_scale = texture2D(FoamMap, UV).r;
//                
//    vec3 reflectDir = reflect(-V, N);     
//                
//    // ���������������Ҫ��Z��Ϊ�Ϸ��򣬵�����պе���ͼ����Y��Ϊ�Ϸ�������Ҫ�Ѳ���������x����ת90��
//    mat3 rotation = mat3(
//        1, 0, 0, 
//        0, 0, -1, 
//        0, 1, 0
//    );
//    vec4 rgbm = texture(skybox, rotation*reflectDir);
//    vec3 sky = rgbm.rgb;
//                
////                //������
////                float fresnel = saturate(_FresnelScale + (1 - _FresnelScale) * pow(1 - dot(N, V), 5));
////                
////                float facing = saturate(dot(V, N));                
////                vec3 oceanColor = lerp(_OceanColorShallow, _OceanColorDeep, facing);
////                
//////                vec3 ambient = UNITY_LIGHTMODEL_AMBIENT.rgb;
////                //������ɫ
////                vec3 oceanDiffuse = oceanColor * _LightColor0.rgb * saturate(dot(L, N));
////                vec3 specular = _LightColor0.rgb * pow(max(0, dot(N, H)), _Gloss);
////                
////                vec3 diffuse = lerp(oceanDiffuse, bubblesDiffuse, bubbles);
////                
//////                vec3 col = ambient + lerp(diffuse, sky, fresnel) + specular ;
////                FragColor = vec4(lerp(diffuse, sky, fresnel) + specular, 1);
//////                FragColor = vec4(vec3(bubbles), 1);
//                
//                
//    vec3 color = vec3(0.0f);
//    float roughness = 0.10;
//    // ����߹�
//    float F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
//    float G = GeometrySmith(NdotL, NdotV, roughness);
//    float NDF = NDF_GGXTR(N, H, roughness);
//    float nominator = NDF * G * F;
//    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
//    float I_sun = nominator / denominator;
//
////    float zL = dot(L, N);
////    float zV = dot(V, N);
////    float zH = dot(H, N);
////    float zH2 = zH * zH;
////
////    vec3 Ty = normalize(vec3(0, N.z, -N.y));
////    vec3 Tx = cross(Ty, N);
////
////    vec2 zetaH = -vec2(dot(H, Tx), dot(H, Ty)) / dot(H, N);
////
////    float cosThetaV = dot(V, N);
////    float cosThetaL = dot(L, N);
////
////    float phiV = atan(dot(V, Ty), dot(V, Tx));
////    float phiL = atan(dot(L, Ty), dot(L, Tx));
////
//////    float sigmaXsq = 0.0137753370;
//////    float sigmaYsq = 0.0212464331;
////    float sigmaXsq = 0.004 + mipmapLevel * 0.0003;
////    float sigmaYsq = 0.004 + mipmapLevel * 0.0003;
//////    float sigmaXsq = sigmaSq.x;
//////    float sigmaYsq = sigmaSq.y;
////
//////    float sigmaXsq = 
//////        imageLoad(SlopeVariance, ivec2(0, 0)).x + imageLoad(SlopeVariance, ivec2(0, 1)).x + 
//////        imageLoad(SlopeVariance, ivec2(1, 0)).x + imageLoad(SlopeVariance, ivec2(1, 1)).x;
////////    sigmaXsq /= float(N) * float(N);
//////    float sigmaYsq = 
//////        imageLoad(SlopeVariance, ivec2(0, 0)).y + imageLoad(SlopeVariance, ivec2(0, 1)).y + 
//////        imageLoad(SlopeVariance, ivec2(1, 0)).y + imageLoad(SlopeVariance, ivec2(1, 1)).y;
//////    sigmaYsq /= float(N) * float(N);
////    vec2 sigmaSq = vec2(sigmaXsq, sigmaYsq);
////
////    float sigmaV = sqrt(sigmaXsq * cos(phiV) * cos(phiV) + sigmaYsq * sin(phiV) * sin(phiV));
////
////    float zetaHx = dot(H, Tx) / dot(H, N);
////    float zetaHy = dot(H, Ty) / dot(H, N);
////    float p = NDF_GGXTR(N, H, 0.12);//P(vec2(zetaHx, zetaHy), sigmaXsq, sigmaYsq);
////
////    float fresnel = F0 + (1-F0) * pow(1.0 - dot(V, H), 5.0);
////
////    float tanV = atan(dot(V, Ty), dot(V, Tx));
////    float cosV2 = 1.0 / (1.0 + tanV * tanV);
////    float sigmaV2 = sigmaSq.x * cosV2 + sigmaSq.y * (1.0 - cosV2);
////
////    float tanL = atan(dot(L, Ty), dot(L, Tx));
////    float cosL2 = 1.0 / (1.0 + tanL * tanL);
////    float sigmaL2 = sigmaSq.x * cosL2 + sigmaSq.y * (1.0 - cosL2);
////
////    float I_sun = fresnel * p / ((1.0 + Lambda(zL, sigmaL2) + Lambda(zV, sigmaV2)) * zV * zH2 * zH2 * 4.0);
//    color += saturate(I_sun) * lights[0].lightColor.rgb * lights[0].lightIntensity;
//
//    // ��շ��䣬����ʹ���˷�����������պн��в�����Ҳ����������Ļ�ռ䷴��ȼ����Ľ�
////    float mean_fresnel = F0 + (1-F0) * meanFresnel(cosThetaV, sigmaV);
//    vec3 I_sky = sky * F;
//    color += saturate(I_sky);
//
//    // ��ˮ����
//    vec3 I_sea = SeaColor * (1-F);
//    color += saturate(I_sea);
//
//    // �α���ɢ��
//    float v = abs(V.z);
//    float sssIntensity = frag_position.z / A / 2;     // �ò�����ģ��α���ɢ���ǿ�ȣ�����һ�ֽ��Ƶ�ģ�⣬����Ч������
//    float towardsSun = pow(max(0., dot(L, -V)), SubSurfaceSunFallOff);
//	vec3 subsurface = (SubSurfaceBase + SubSurfaceSun * towardsSun) * SubSurfaceColour.rgb * lights[0].lightColor.rgb;
//	subsurface *= (1.0 - v * v) * sssIntensity;
//	color += subsurface;
//
//    //��ĭ��ɫ������û�ҵ����ʵ���ͼ�زģ���ĭֻ����FoamColor�ջ�һ��
//    vec3 foamDiffuse = FoamColor.rbg * saturate(dot(L, N));// * lights[0].lightColor.rgb;
//    color += foam_scale * foamDiffuse;
//
////    FragColor = vec4(vec3(mean_fresnel), 1);
////    float mipmapLevel = textureQueryLod(FoamMap, UV).x;
//    FragColor = vec4(hdr(color), 1);
////    FragColor = vec4(vec3(mipmapLevel) / 6, 1);
////    FragColor = vec4(vec3(frag_position.z)/2, 1);
//    
//    /*******������ʾLOD*******/
////    if (-0.5 < LOD && LOD < 0.5)
////        color = vec3(0, 1, 0);
////    else if (0.5 < LOD && LOD < 1.5)
////        color = vec3(0, 0, 1);
////    else if (1.5 < LOD && LOD < 2.5)
////        color = vec3(1, 0, 0);
////    else if (2.5 < LOD && LOD < 3.5)
////        color = vec3(1, 1, 0);
////    else if (3.5 < LOD && LOD < 4.5)
////        color = vec3(0, 1, 1);
////    else if (4.5 < LOD && LOD < 5.5)
////        color = vec3(1, 0, 1);
////    else if (5.5 < LOD && LOD < 6.5)
////        color = vec3(1, 1, 1);
////    FragColor = vec4(color, 1);
//    /************************/
//
//    /*******������ʾLOD��ӷ�*******/
////    color = vec3(1, 1, 1);
////    FragColor = vec4(color, 1);
//    /*****************************/
//
////    FragColor = vec4(UV, 0, 1);
////    float Kd = 0.6f;
////    float Ks = 1.0f;
////    float Ka = 0.2f;
////    float shininess = 30;
////    vec3 objectColor = vec3(0.13,0.2,0.26);
////    vec3 norm = normalize(texture(NormalMap, UV)).xyz;
////    vec3 V = normalize(cameraPos - frag_position);
////    vec3 LightDir = normalize(-lights[0].lightDirection);
////    vec3 R = -LightDir - norm * (2.0 * dot(norm, -LightDir));
////    float LN = dot(norm, LightDir);
////    float RV = dot(R, V);
////    vec3 Id = vec3(lights[0].lightColor) * Kd * max(0, LN);
////    vec3 Is = vec3(lights[0].lightColor) * Ks * pow(max(0, RV), shininess);
////    vec3 Ia = vec3(lights[0].lightColor) * Ka;
////    FragColor = 3 * vec4((Ia + Id + Is) * objectColor, 1);
//    //FragColor = vec4(texture(FoamMap, UV).rgb, 1);
//}