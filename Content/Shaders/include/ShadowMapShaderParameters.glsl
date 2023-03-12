#ifndef SHADOWMAPSHADERPARAMETERS_H
#define SHADOWMAPSHADERPARAMETERS_H

struct FShadowMappingParameters
{
    dmat4 WorldToClip;
    ivec2 Resolution;
    float FrustumFar;
};

//#ifndef FrustumCount
//#define FrustumCount (1)
//#endif

#endif