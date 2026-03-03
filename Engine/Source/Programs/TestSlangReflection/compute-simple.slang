// compute-simple.slang

static const uint THREADGROUP_SIZE_X = 8;
static const uint THREADGROUP_SIZE_Y = THREADGROUP_SIZE_X;

struct ImageProcessingOptions
{
    float3 tintColor;
    float blurRadius;

    bool useLookupTable;
    StructuredBuffer<float4> lookupTable;
}
struct LayoutTestStruct
{
    float array[3];
};
uniform ParameterBlock<LayoutTestStruct>   ConstantBuffer1;
uniform ParameterBlock<LayoutTestStruct>   ConstantBuffer2;
uniform StructuredBuffer<LayoutTestStruct>   StructuredBuffer1;


[shader("compute")]
[numthreads(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y)]
void processImage(
    uint3 threadID : SV_DispatchThreadID,
    uniform Texture2D inputImage,
    uniform RWTexture2D outputImage,
    uniform ImageProcessingOptions options)
{
    outputImage[0] = ConstantBuffer1.array[0] + ConstantBuffer2.array[1] + StructuredBuffer1[0].array[2];
    /* actual logic would go here */
}
