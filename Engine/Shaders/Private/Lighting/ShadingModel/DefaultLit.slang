implementing LightingPassPixelShader;

float3 ApplyDefaultLit(Light light, ShadingParameters params, float visibility)
{
    float3 baseColor = params.baseColor.rgb;
    float3 emissive  = params.emissive.rgb;
    float3 N         = params.N;
    float3 L         = params.L;
    float3 H         = params.H;
    float3 V         = params.V;

    float  NdotL        = max(dot(N, L), 0.0f);
    float  NdotV        = max(dot(N, V), 0.0f);
    float3 F0           = lerp(float3(0.04f), baseColor, params.metallic);
    float3 F            = fresnel_Schlick(F0, clamp(dot(H, L), 0.0f, 1.0f));
    float  G            = GeometrySmith(NdotL, NdotV, params.roughness);
    float  NDF          = NDF_GGXTR(N, H, params.roughness);
    float3 nominator    = NDF * G * F;
    float  denominator  = 4.0f * max(NdotV, 0.0f) * max(NdotL, 0.0f) + 0.001f;
    float3 BRDF         = nominator / denominator;

    float atten = 1.0f;
    if (light.type == ELightType.Point || light.type == ELightType.Spot)
    {
        float3 RelativeLightPosition = float3(light.position - ViewParameters.CameraPosition);
        float dist = length(RelativeLightPosition);
        atten *= light.distAttenCurve.ApplyAttenCurve(dist);
    }
    if (light.type == ELightType.Spot)
    {
        float angle = acos(dot(normalize(light.direction), -L));
        atten *= light.angleAttenCurve.ApplyAttenCurve(angle);
    }
    float3 radiance = atten * light.intensity;

    float3 kS = F;
    float3 kD = float3(1.0f) - kS;
    kD *= 1.0f - params.metallic;

    float3 color = (kD * baseColor / PI + BRDF) * radiance * NdotL * visibility + emissive;
    return color;
}
