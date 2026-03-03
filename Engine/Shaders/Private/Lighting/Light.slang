implementing LightingPassPixelShader;

enum ELightType
{
    None = 0,
    Spot = 1,
    Directional = 2,
    Point = 3
};

enum ELightAttenuationCurveType
{
    None = 0,
    Linear = 1,
    Smooth = 2,
    Inverse = 3,
    InverseSquare = 4
};

struct LightAttenuationCurve
{
    float4 params;
    float  scale;
    ELightAttenuationCurveType type;

    float ApplyAttenCurve(float t)
    {
        float  atten  = 1.0f;
        switch (type)
        {
            case ELightAttenuationCurveType.Linear: // Linear
                atten = LinearAtten(t, scale, params.x, params.y);
                break;
            case ELightAttenuationCurveType.Smooth: // Smooth
                atten = SmoothAtten(t, scale, params.x, params.y);
                break;
            case ELightAttenuationCurveType.Inverse: // Inverse
                atten = InverseAtten(t, scale, params.x, params.y, params.z);
                break;
            case ELightAttenuationCurveType.InverseSquare: // InverseSquare
                atten = InverseSquareAtten(t, scale, params.x, params.y, params.z, params.w);
                break;
            case ELightAttenuationCurveType.None: // None
                break;
            default:
                break;
        }
        return atten;
    }
};

struct Light
{
    LightAttenuationCurve distAttenCurve;
    LightAttenuationCurve angleAttenCurve;
    float3 position;
    float3 intensity;
    float3 direction;
    ELightType type;
    bool castShadow;
};

float LinearAtten(float t, float scale, float begin_atten, float end_atten)
{
    if (t < begin_atten)
    {
        return 1.0f * scale;
    }
    else if (t > end_atten)
    {
        return 0.0f;
    }
    else
    {
        return (end_atten - t) / (end_atten - begin_atten) * scale;
    }
}

float SmoothAtten(float t, float scale, float begin_atten, float end_atten)
{
    float tmp   = LinearAtten(t, scale, begin_atten, end_atten);
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
    float atten = clamp(pow(scale, 2.0f) / (kq * pow(t, 2.0f) + kl * t * scale + kc * pow(scale, 2.0f)) + offset,
        0.0f, 1.0f);
    return atten;
}
