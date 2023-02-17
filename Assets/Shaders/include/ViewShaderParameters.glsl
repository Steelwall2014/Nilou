#ifndef VIEWSHADERPARAMETERS_H
#define VIEWSHADERPARAMETERS_H
layout (std140) uniform FViewShaderParameters {
    mat4 RelWorldToView;
    mat4 ViewToClip;
    mat4 RelWorldToClip;
    mat4 ClipToView;
    mat4 RelClipToWorld;
    mat4 AbsWorldToClip;
    dvec3 CameraPosition;
    vec3 CameraDirection;
    ivec2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
};
#endif