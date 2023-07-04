#pragma once

#include "RHIResources.h"

namespace nilou {

class FPipelineStateCache
{
public:
    static void CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, FRHIGraphicsPipelineStateRef CacheState);
    static FRHIGraphicsPipelineState* FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer);
    static void ClearCacheGraphicsPSO() { GraphicsPipelineCache.clear(); }
    static FRHIVertexDeclaration* GetOrCreateVertexDeclaration(const FVertexDeclarationElementList& Elements);
    static void ClearCacheVertexDeclarations() { VertexDeclarationCache.clear(); }
private:
    static std::unordered_map<FGraphicsPipelineStateInitializer, FRHIGraphicsPipelineStateRef> GraphicsPipelineCache;
    static std::unordered_map<uint32, FRHIVertexDeclarationRef> VertexDeclarationCache;
};

}