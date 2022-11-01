#version 460 core
const float PI = 3.14159f;
in vec2 UV;
in vec3 frag_position;
in float LOD;
in float OnEdge;

in vec2 Texture_UV;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 SonarHUDColor;

uniform vec3 cameraPos;
uniform float MaxLOD;
uniform sampler2D HeightMap;
uniform sampler2D BaseNormal;

#include "../include/Light.glsl"
#include "../include/PBRFunctions.glsl"
//#include "include/functions.glsl"
//#include "waterbody/waterbody_definitions.glsl"
//#include "waterbody/waterbody_functions.glsl"

uniform sampler2D baseColorMap;
//uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
//uniform sampler2D occlusionMap;
uniform sampler2D roughnessMap;


uniform sampler2D causticMap;
uniform float Time;
uniform vec2 WindDirection;

vec3 PBR_without_radiance(vec3 L, vec3 N, vec3 V, vec3 baseColor, float roughness, float metallic)
{
    vec3 H = normalize(L+V);
    float NdotL = clamp(dot(N, L), 0.0, 1.0); 
    float NdotV = clamp(dot(N, V), 0.0, 1.0); 
    float HdotL = clamp(dot(H, L), 0.0, 1.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, metallic);
    vec3 F = fresnel_Schlick(F0, HdotL);
    float G = GeometrySmith(NdotL, NdotV, roughness);
    float NDF = NDF_GGXTR(N, H, roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
    vec3 BRDF = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;  

    return (kD * baseColor / PI + BRDF) * NdotL;
}

vec3 hdr(vec3 L, float hdrExposure) {
    vec3 mapped = vec3(1.0) - exp(-L * hdrExposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    return mapped;
}

void main()
{
	vec3 base_normal = texture(BaseNormal, Texture_UV).rgb;
	vec3 tangent = normalize(vec3(base_normal.z, 0, -base_normal.x));
	vec3 bitangent = normalize(vec3(0, base_normal.z, -base_normal.y));
	mat3 TBN = mat3(tangent, bitangent, base_normal);

	vec3 L = -refract(lights[0].lightDirection, vec3(0, 0, 1), 1 / 1.333);
    float dist = length(frag_position-cameraPos);
    vec3 V = normalize(cameraPos - frag_position);
    vec3 tangentspace_normal = texture(normalMap, Texture_UV).rgb;
    tangentspace_normal = normalize(tangentspace_normal * 2 - 1);
    vec3 N = normalize(TBN * tangentspace_normal);
    vec3 baseColor = texture(baseColorMap, Texture_UV).rgb;
    float metallic = 0;
    float roughness = max(0.3, texture(roughnessMap, Texture_UV).r);
    vec3 color = PBR_without_radiance(L, N, V, baseColor, roughness, metallic);
    
    float caustic = texture(causticMap, Texture_UV - WindDirection * Time * 0.1f).r*5;

    FragColor = vec4(color * caustic, 1);

    if (mod(frag_position.z, 5) > 9.95 || mod(frag_position.z, 5) < 0.05)
        color = vec3(0.7);
    else
        color = vec3(0.1);
    vec3 ambient = vec3(0.1);
    vec3 norm = normalize(base_normal);
    float diff = max(dot(norm, L), 0.0);
    vec3 diffuse = diff * vec3(1) * color;
    SonarHUDColor = vec4(diffuse, 1);
//    vec3 view_direction = normalize(frag_position-cameraPos);
//    const int SAMPLE_NUM = 50;
//    float dx = dist / float(SAMPLE_NUM);
//    vec3 radiance = vec3(0.f);
//    for (float i = 0.5; i < SAMPLE_NUM; i += 1)
//    {
//        vec3 P = cameraPos + view_direction * dx * i;
//        vec3 C = P + L * (abs(P.z) / L.z);
//        vec3 transmittance = Transmittance(WATERBODY, P, cameraPos) * Transmittance(WATERBODY, C, P);
//        float cosTheta = dot(L, view_direction);
//        vec3 phase = Phase(WATERBODY, cosTheta);
//        radiance += transmittance * phase * WATERBODY.total_scattering * dx;
//    }
//    vec3 C = frag_position + L * (abs(frag_position.z) / L.z);
//    FragColor = vec4(radiance * 10, 1.f);
//    return;
//    vec3 lightColor = vec3(1);
//    float ambientStrength = 0.1;
//    vec3 ambient = ambientStrength * lightColor;
//  	
//    // diffuse 
////	vec3 lightPos = normalize(-lights[0].lightDirection);
//    vec3 norm = normalize(frag_normal);
//    vec3 lightDir = normalize(-lights[0].lightDirection);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * lightColor;
//    
//    // specular
//    float specularStrength = 0.5;
//    vec3 viewDir = normalize(cameraPos - frag_position);
//    vec3 reflectDir = reflect(-lightDir, norm);  
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//    vec3 specular = specularStrength * spec * lightColor;  
//        
//    vec3 result = (ambient + diffuse + specular) * vec3(1);
//    FragColor = vec4(result, 1.0);
//    vec3 color = vec3(0.7);
//    int h = int(round(frag_position.z));    
//    if (h % 10 > 9 || h % 10 < 1)
//        color = vec3(0.7);
//    else
//        color = vec3(0.1);
//    for (int i = 0; i < lightCount; i++)
//    {
//        if (lights[i].lightCastShadow)
//        {
//            color += apply_light(lights[i], frag_lightspace_pos[i]);
//        }
//    }
//    FragColor = vec4(color, 1);
    
//    color = vec3(0.8, 0.8, 0.8);
//    FragColor = vec4(color, 1);
}