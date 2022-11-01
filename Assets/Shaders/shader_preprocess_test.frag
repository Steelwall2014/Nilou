#version 450 core
#define PI 3.1415926
layout (location = 0) out vec4 FragColor;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 frag_lightspace_pos[5];
in vec2 uv;
in mat3 TBN;

uniform vec3 cameraPos;

//uniform sampler2D shadowMap;
uniform sampler2DArray shadowMap;
uniform sampler2D baseColorMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
uniform sampler2D occlusionMap;
uniform sampler2D roughnessMetallicMap;

layout(std140, binding = 0) uniform Matrix {
    mat4 m1;
    mat4 m2;
};

layout(binding = 2) uniform sampler2D aaa;
uniform sampler2D bbb;
uniform int ccc;

#include "include/Light.glsl"
#include "include/PBRFunctions.glsl"


//vec3  albedo = vec3(0.0, 0.6, 1);
//float metallic = 0.8;
//float roughness = 0.15;
float ao = 1.0;
vec3 baseColor;
float metallic;
float roughness;



float ShadowCalculation(vec4 fragPosLightSpace, float layer_index, float bias)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(shadowMap, vec3(projCoords.xy, layer_index)).r; 
//    //float closestDepth = texture(shadowMap, vec3(projCoords.xy, layer_index)).r; 
    float currentDepth = projCoords.z;
    const vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 )
    );
    float visibility = 1.f;// = currentDepth-bias > closestDepth  ? 0.0 : 1.0;
    for (int i = 0; i < 4; i++)
    {
        float closestDepth = texture(shadowMap, vec3(projCoords.xy + poissonDisk[i] / 700.0f, layer_index)).r;
        if (currentDepth - bias > closestDepth)
        {
            visibility -= 0.25f;
        }
    }
    return visibility;
}

vec3 apply_light(const Light light, const vec4 fragPosLightSpace, const vec3 frag_position, vec3 N, vec3 V)
{
    vec3 L;
    if (light.lightType == 2)   // Directional
        L = normalize(-light.lightDirection);
    else 
        L = normalize(light.lightPosition - frag_position);

    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0); 
    float NdotV = max(dot(N, V), 0.0); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, metallic);
    vec3 F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
    float G = GeometrySmith(NdotL, NdotV, roughness);
    float NDF = NDF_GGXTR(N, H, roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
    vec3 BRDF = nominator / denominator;

    float bias = max(0.01 * (1.0 - NdotL), 0.001);
    float visibility = ShadowCalculation(fragPosLightSpace, light.lightShadowMapLayerIndex, bias);

    float atten = 1;
    if (light.lightType == 0 || light.lightType == 1)   // Point or Spot
    {
        float dist = length(light.lightPosition - frag_position);
        atten *= apply_atten_curve(dist, light.lightDistAttenCurveType, light.lightDistAttenCurveParams);
    }
    if (light.lightType == 1)       // Spot 
    {
        float angle = acos(dot(normalize(light.lightDirection), -L));
        atten *= apply_atten_curve(angle, light.lightAngleAttenCurveType, light.lightAngleAttenCurveParams);
    } 
    vec3 radiance = atten * vec3(light.lightColor) * light.lightIntensity;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;  

    return (kD * baseColor / PI + BRDF) * radiance * NdotL * visibility;
//    return atten * (Ia + visibility * Id + visibility * Is) * objectColor;
}

void main()
{
    vec3 color = vec3(0.f);
    vec3 tangent_normal = texture(normalMap, uv).rgb;
    vec3 N;
    if (tangent_normal == vec3(0.f))
    {
        N = normalize(frag_normal);
    }
    else
    {
        tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);   
        N = normalize(TBN * tangent_normal);
    }
    //N = normalize(frag_normal);
    vec3 V = normalize(cameraPos - frag_position);

    baseColor = texture(baseColorMap, uv).rgb;

    metallic = texture(roughnessMetallicMap, uv).b;

    roughness = texture(roughnessMetallicMap, uv).g;

    for (int i = 0; i < lightCount; i++)
    {
        if (lights[i].lightCastShadow)
        {
            color += apply_light(lights[i], frag_lightspace_pos[i], frag_position, N, V);
        }
    }    
    color += texture(emissiveMap, uv).rgb;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
    FragColor = vec4(color, 1);
}