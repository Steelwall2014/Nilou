#pragma once
#include "HAL/Platform.h"
#include "RenderGraph.h"

namespace nilou {

class ENGINE_API FGenerateMips
{
public:
    static void Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=nullptr);
    static void ExecuteCompute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=nullptr);
    static void ExecuteRaster(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler=nullptr);
};

}