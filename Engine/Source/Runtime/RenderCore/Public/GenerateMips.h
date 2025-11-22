#pragma once
#include "HAL/Platform.h"
#include "RenderGraph.h"

namespace nilou {

class RENDERCORE_API FGenerateMips
{
public:
    static void Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=TStaticSamplerState<SF_Bilinear>::GetRHI());
    static void ExecuteCompute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=TStaticSamplerState<SF_Bilinear>::GetRHI());
    static void ExecuteRaster(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=TStaticSamplerState<SF_Bilinear>::GetRHI());
};

}