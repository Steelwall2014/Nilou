#ifndef VIEWSHADERPARAMETERS_H
#define VIEWSHADERPARAMETERS_H
layout (std140) uniform FViewShaderParameters {
    mat4 WorldToView;
    mat4 ViewToClip;
    mat4 WorldToClip;
    mat4 ClipToWorld;
    dvec3 CameraPosition;
    vec3 CameraDirection;
    ivec2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
};
#endif