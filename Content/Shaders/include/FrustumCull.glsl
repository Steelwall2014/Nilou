

bool IsOutSidePlane(vec4 plane, vec3 position){
    return dot(plane.xyz,position) + plane.w < 0; 
}
bool IsBoxOutSidePlane(vec4 plane, vec3 box[8])
{
    return IsOutSidePlane(plane, box[0]) && 
    IsOutSidePlane(plane, box[1]) &&
    IsOutSidePlane(plane, box[2]) &&
    IsOutSidePlane(plane, box[3]) &&
    IsOutSidePlane(plane, box[4]) &&
    IsOutSidePlane(plane, box[5]) &&
    IsOutSidePlane(plane, box[6]) &&
    IsOutSidePlane(plane, box[7]);
}
bool FrustumCull(vec3 box[8])
{
    return IsBoxOutSidePlane(vec4(FrustumPlanes[0]), box) || 
    IsBoxOutSidePlane(vec4(FrustumPlanes[1]), box) || 
    IsBoxOutSidePlane(vec4(FrustumPlanes[2]), box) || 
    IsBoxOutSidePlane(vec4(FrustumPlanes[3]), box) || 
    IsBoxOutSidePlane(vec4(FrustumPlanes[4]), box) || 
    IsBoxOutSidePlane(vec4(FrustumPlanes[5]), box);
}

void CreateBox(vec2 upper_left, vec2 lower_right, vec2 MinMax, out vec3 box[8])
{
    vec3 center = (vec3(upper_left, MinMax.x) + vec3(lower_right, MinMax.y)) / 2.0f;
    upper_left = (upper_left - center.xy) * 2 + center.xy;
    lower_right = (lower_right - center.xy) * 2 + center.xy;
    MinMax = (MinMax - vec2(center.z)) * 2 + vec2(center.z);
    box[0] = vec3(upper_left.x, upper_left.y, MinMax.r);
    box[1] = vec3(lower_right.x, upper_left.y, MinMax.r);
    box[2] = vec3(upper_left.x, lower_right.y, MinMax.r);
    box[3] = vec3(lower_right.x, lower_right.y, MinMax.r);
    box[4] = vec3(upper_left.x, upper_left.y, MinMax.g);
    box[5] = vec3(lower_right.x, upper_left.y, MinMax.g);
    box[6] = vec3(upper_left.x, lower_right.y, MinMax.g);
    box[7] = vec3(lower_right.x, lower_right.y, MinMax.g);
}