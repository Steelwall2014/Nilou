#version 330 core

layout (location = 0) out vec4 FragColor;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 frag_lightspace_pos[5];

//uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 cameraPos;
//uniform vec3 lightPos;
uniform float Kd;
uniform float Ks;
uniform float Ka;
uniform float shininess;

//uniform sampler2D shadowMap;
uniform sampler2DArray shadowMap;
struct Light
{
        vec3       lightPosition;
        vec4       lightColor;
        vec4       lightDirection;
        float      lightIntensity;
        bool       lightCastShadow;
        int        lightShadowMapLayerIndex;
        mat4       lightVP;
};
uniform Light lights[5];
uniform int lightCount;

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
            visibility -= 0.2f;
        }
    }
    return visibility;
}

float LinearAtten(float t, float begin, float end)
{
    if (t < begin)
    {
        return 1.0f;
    }
    else if (t > end)
    {
        return 0.0f;
    }
    else
    {
        return (end - t) / (end - begin);
    }
}
float InverseSquareAtten(float dist)
{
    float scale = 10.f;
    float offset = 0.7f;
    float kq = 1.f;
    float kl = 0.f;
    float kc = 0.f;
    float atten = clamp(pow(scale, 2.0f) / (kq * pow(dist, 2.0f) + kl * dist * scale + kc * pow(scale, 2.0f)) + offset, 
        0.0f, 1.0f);
    return atten;
}
float SmoothAtten(float angle)
{
    float begin_atten = 0.334917597999862;
    float end_atten = 0.5427974462509155;
    float tmp = LinearAtten(angle, begin_atten, end_atten);
    float atten = 3.0f * pow(tmp, 2.0f) - 2.0f * pow(tmp, 3.0f);
    return atten;
}

vec3 apply_light(const Light light, const vec4 fragPosLightSpace)
{
    vec3 light_pointing = normalize(light.lightPosition - vec3(0, 0, 0));
    vec3 norm = normalize(frag_normal);
    vec3 V = normalize(cameraPos - frag_position);
    vec3 LightDir = normalize(light.lightPosition - frag_position);

    float atten = 1.0f;
    float dist = distance(light.lightPosition, frag_position);
    float angle = acos(dot(light_pointing, LightDir));
    atten *= InverseSquareAtten(dist);
    atten *= SmoothAtten(angle);

    vec3 R = -LightDir - norm * (2.0 * dot(norm, -LightDir));
    float LN = dot(norm, LightDir);
    float RV = dot(R, V);
    vec3 Id = vec3(light.lightColor) * Kd * max(0, LN);
    vec3 Is = vec3(light.lightColor) * Ks * pow(max(0, RV), shininess);
    vec3 Ia = vec3(light.lightColor) * Ka;
    
    float bias = max(0.01 * (1.0 - LN), 0.001);
    float visibility = ShadowCalculation(fragPosLightSpace, light.lightShadowMapLayerIndex, bias);
    return atten * (Ia + visibility * Id + visibility * Is) * objectColor;
}

void main()
{
    vec3 color;
    for (int i = 0; i < lightCount; i++)
    {
        if (lights[i].lightCastShadow)
        {
            color += apply_light(lights[i], frag_lightspace_pos[i]);
        }
    }
    FragColor = vec4(color, 1);
//    vec3 projCoords = frag_lightspace_pos[0].xyz / frag_lightspace_pos[0].w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(shadowMap, vec3(projCoords.xy, 0)).r; 
//    FragColor = vec4(closestDepth, closestDepth, closestDepth, 1);
}