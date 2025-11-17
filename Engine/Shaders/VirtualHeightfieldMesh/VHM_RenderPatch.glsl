#ifndef VHM_RENDERPATCH_H
#define VHM_RENDERPATCH_H

struct RenderPatch
{
    uint DeltaLod_x_pos;
    uint DeltaLod_x_neg;
    uint DeltaLod_y_pos;
    uint DeltaLod_y_neg;
    float offset_x;
    float offset_y;
    uint lod;
};

#endif