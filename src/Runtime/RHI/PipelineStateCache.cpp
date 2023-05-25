#include "PipelineStateCache.h"

namespace nilou {

void FPipelineStateCache::CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, FRHIGraphicsPipelineStateRef CacheState)
{
    GraphicsPipelineCache[Initializer] = CacheState;
}

FRHIGraphicsPipelineState* FPipelineStateCache::FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer)
{
    if (!GraphicsPipelineCache.contains(Initializer))
        return nullptr;
    return GraphicsPipelineCache[Initializer].get();
}

std::unordered_map<FGraphicsPipelineStateInitializer, FRHIGraphicsPipelineStateRef> 
FPipelineStateCache::GraphicsPipelineCache{};

}