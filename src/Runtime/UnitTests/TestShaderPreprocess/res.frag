layout(std140) uniform  Matrix {
    mat4 m1;
    mat4 m2;
};
uniform sampler2D aaa;

uniform sampler2D baseColorMap;

uniform sampler2D bbb;

uniform vec3 cameraPos;

uniform int ccc;

uniform sampler2D emissiveMap;

uniform sampler2D normalMap;

uniform sampler2D occlusionMap;

uniform sampler2D roughnessMetallicMap;

uniform sampler2D shadowMap;

#define PI 3.1415926
layout (location = 0) out vec4 FragColor;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 frag_lightspace_pos[5];
in vec2 uv;
in mat3 TBN;


//

struct Light
{
        float      lightDistAttenCurveParams[5];
        float      lightAngleAttenCurveParams[5];
        mat4       lightVP;
        vec4       lightColor;
        vec3       lightPosition;
        vec3       lightDirection;
        int        lightType;
        float      lightIntensity;
        int        lightDistAttenCurveType;
        int        lightAngleAttenCurveType;
        bool       lightCastShadow;
        int        lightShadowMapLayerIndex;
};

float LinearAtten(float t, float begin_atten, float end_atten)
{
    if (t < begin_atten)
    {
        return 1.0f;
    }
    else if (t > end_atten)
    {
        return 0.0f;
    }
    else
    {
        return (end_atten - t) / (end_atten - begin_atten);
    }
}
float SmoothAtten(float t, float begin_atten, float end_atten)
{
    float tmp = LinearAtten(t, begin_atten, end_atten);
    float atten = 3.0f * pow(tmp, 2.0f) - 2.0f * pow(tmp, 3.0f);
    return atten;
}
float InverseAtten(float t, float scale, float offset, float kl, float kc)
{
    float atten = clamp((scale / ((kl * t) + (kc * scale))) + offset, 0.0f, 1.0f);
    return atten;
}
float InverseSquareAtten(float t, float scale, float offset, float kq, float kl, float kc)
{
    float atten = clamp(pow(scale, 2.0f) / (kq * pow(t, 2.0f) + kl*t*scale + kc*pow(scale, 2.0f)) + offset, 
        0.0f, 1.0f);
    return atten;
}

float apply_atten_curve(float t, int atten_curve_type, float atten_params[5])
{
    float atten = 1.f;
    switch (atten_curve_type)
    {
    case 1:     // Linear
        atten = LinearAtten(t, atten_params[0], atten_params[1]);
        break;
    case 2:     // Smooth
        atten = SmoothAtten(t, atten_params[0], atten_params[1]);
        break;
    case 3:     // Inverse
        atten = InverseAtten(t, atten_params[0], atten_params[1], atten_params[2], atten_params[3]);
        break;
    case 4:     // InverseSquare
        atten = InverseSquareAtten(t, atten_params[0], atten_params[1], atten_params[2], atten_params[3], atten_params[4]);
        break;
    case 0:     // None
        break;
    default:
        break;
    }
    return atten;
}
vec3 fresnel_Schlick(vec3 R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}
float fresnel_Schlick(float R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}
float NDF_GGXTR(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // SIGGRAPH 2013£ºUE4
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float NdotL, float NdotV, float roughness)
{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}


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
