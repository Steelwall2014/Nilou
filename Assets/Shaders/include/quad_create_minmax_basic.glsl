layout(rg32f, binding=1) uniform image2D InMinMaxMap;
layout(rg32f, binding=2) uniform image2D OutMinMaxMap;

void main()
{
    ivec2 in_patchLoc = ivec2(gl_GlobalInvocationID.xy) * 2;

    float min_heights[4];
    min_heights[0] = imageLoad(InMinMaxMap, in_patchLoc).r;
    min_heights[1] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(0, 1)).r;
    min_heights[2] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(1, 0)).r;
    min_heights[3] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(1, 1)).r;

    float max_heights[4];
    max_heights[0] = imageLoad(InMinMaxMap, in_patchLoc).g;
    max_heights[1] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(0, 1)).g;
    max_heights[2] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(1, 0)).g;
    max_heights[3] = imageLoad(InMinMaxMap, in_patchLoc + ivec2(1, 1)).g;

    float min_height = min(min_heights[0], min(min_heights[1], min(min_heights[2], min_heights[3])));
    float max_height = max(max_heights[0], max(max_heights[1], max(max_heights[2], max_heights[3])));
    imageStore(OutMinMaxMap, ivec2(gl_GlobalInvocationID.xy), vec4(min_height, max_height, 0, 0));
}