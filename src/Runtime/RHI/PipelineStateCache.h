#pragma once

#include "RHIResources.h"

namespace nilou {

class FPipelineStateCache
{
public:
    static void CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, RHIGraphicsPipelineStateRef CacheState);
    static RHIGraphicsPipelineState* FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer);
    static void ClearCacheGraphicsPSO() { GraphicsPipelineCache.clear(); }
    static void CacheComputePSO(RHIComputeShader* ComputeShader, RHIComputePipelineStateRef CacheState);
    static RHIComputePipelineState* FindCachedComputePSO(RHIComputeShader* ComputeShader);
    static void ClearCacheComputePSO() { ComputePipelineCache.clear(); }
    static void CacheVertexDeclaration(const FVertexDeclarationElementList& ElementList, FRHIVertexDeclarationRef VertexDeclaration);
    static FRHIVertexDeclaration* FindVertexDeclaration(const FVertexDeclarationElementList& Elements);
    static void ClearCacheVertexDeclarations() { VertexDeclarationCache.clear(); }
private:
    static std::unordered_map<FGraphicsPipelineStateInitializer, RHIGraphicsPipelineStateRef> GraphicsPipelineCache;
    static std::unordered_map<RHIComputeShader*, RHIComputePipelineStateRef> ComputePipelineCache;
    static std::unordered_map<uint32, FRHIVertexDeclarationRef> VertexDeclarationCache;
};

}