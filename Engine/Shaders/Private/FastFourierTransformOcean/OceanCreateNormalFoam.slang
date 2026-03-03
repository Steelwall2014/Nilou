module OceanCreateNormalFoam;
import RenderCore;
import OceanFastFourierTransformParameters;

struct FOceanCreateNormalFoamParameters
{
    RWTexture2D<float4> DisplaceRT;
    RWTexture2D<float4> NormalRT;
    RWTexture2D<float> FoamRT;
}
ParameterBlock<FOceanCreateNormalFoamParameters> Params;

void CreateNormalAndFoam(uint3 id)
{
    //获取当前点，周围4个点的uv坐标
    int2 uvX1 = int2((id.x - 1 + OceanFFTParameters.N) % OceanFFTParameters.N, id.y);
    int2 uvX2 = int2((id.x + 1 + OceanFFTParameters.N) % OceanFFTParameters.N, id.y);
    int2 uvY1 = int2(id.x, (id.y - 1 + OceanFFTParameters.N) % OceanFFTParameters.N);
    int2 uvY2 = int2(id.x, (id.y + 1 + OceanFFTParameters.N) % OceanFFTParameters.N);

    //以当前点为中心，获取周围4个点的偏移值
    
    float3 x1_displace = Params.DisplaceRT[uvX1].xyz;//沿x轴 第一个点的偏移值
    float3 x2_displace = Params.DisplaceRT[uvX2].xyz;//沿x轴 第二个点的偏移值
    float3 y1_displace = Params.DisplaceRT[uvY1].xyz;//沿y轴 第一个点的偏移值
    float3 y2_displace = Params.DisplaceRT[uvY2].xyz;//沿y轴 第二个点的偏移值

    //以当前点为原点，计算周围4个点的世界坐标
    float FFTPixelMeterSize = OceanFFTParameters.DisplacementTextureSize / OceanFFTParameters.N;
    float x0 = id.x * FFTPixelMeterSize;
    float y0 = id.y * FFTPixelMeterSize;

    float3 x1 = float3(x0-FFTPixelMeterSize, y0, 0) + x1_displace;//沿x轴 第一个点的世界坐标
    float3 x2 = float3(x0+FFTPixelMeterSize, y0, 0) + x2_displace;//沿x轴 第二个点的世界坐标
    float3 y1 = float3(x0, y0-FFTPixelMeterSize, 0) + y1_displace;//沿y轴 第一个点的世界坐标
    float3 y2 = float3(x0, y0+FFTPixelMeterSize, 0) + y2_displace;//沿y轴 第二个点的世界坐标

    //计算两个切向量
    float3 tangentX = x2 - x1;
    float3 tangentY = y2 - y1;

    //计算法线
    float3 normal = normalize(cross(tangentX, tangentY));
    float2 gradient = float2(x1.z - x2.z, y1.z - y2.z);

    //计算泡沫
    float3 ddx = (x2_displace - x1_displace) / (2 * FFTPixelMeterSize);
    float3 ddy = (y2_displace - y1_displace) / (2 * FFTPixelMeterSize);
    //雅可比行列式
    float jacobian = (1.0f + ddx.x) * (1.0f + ddy.y) - ddx.y * ddy.x;

    // 这里纯粹是调试用的，没有什么实际意义
//    jacobian = saturate(max(0, 0.75 - jacobian)) * 5;
//    jacobian = 1-jacobian;

    Params.NormalRT[id.xy] = float4(gradient, 2 * FFTPixelMeterSize, 0);
    Params.FoamRT[id.xy] = jacobian;
}

[shader("compute")]
[numthreads(32, 32, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
    CreateNormalAndFoam(threadID);
}
