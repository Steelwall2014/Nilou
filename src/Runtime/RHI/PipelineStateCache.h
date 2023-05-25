#pragma once

#include "RHIResources.h"

namespace nilou {

class FPipelineStateCache
{
public:
    static void CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, FRHIGraphicsPipelineStateRef CacheState);
    static FRHIGraphicsPipelineState* FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer);
    static void ClearCacheGraphicsPSO() { GraphicsPipelineCache.clear(); }
private:
    static std::unordered_map<FGraphicsPipelineStateInitializer, FRHIGraphicsPipelineStateRef> GraphicsPipelineCache;
};

}