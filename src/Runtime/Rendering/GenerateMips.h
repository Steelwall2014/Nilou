#pragma once
#include "Platform.h"
#include "RenderGraph.h"

namespace nilou {

class FGenerateMips
{
public:
    static void Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=TStaticSamplerState<SF_Bilinear>::GetRHI());
};

}