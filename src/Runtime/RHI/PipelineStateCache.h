#pragma once

#include "RHIResources.h"

namespace nilou {

class FPipelineStateCache
{
public:
    static void CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, FRHIPipelineStateRef CacheState);
    static FRHIPipelineState* FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer);
    static void ClearCacheGraphicsPSO() { GraphicsPipelineCache.clear(); }
    static void CacheComputePSO(RHIComputeShader* ComputeShader, FRHIPipelineStateRef CacheState);
    static FRHIPipelineState* FindCachedComputePSO(RHIComputeShader* ComputeShader);
    static void ClearCacheComputePSO() { ComputePipelineCache.clear(); }
    static void CacheVertexDeclaration(const FVertexDeclarationElementList& ElementList, FRHIVertexDeclarationRef VertexDeclaration);
    static FRHIVertexDeclaration* FindVertexDeclaration(const FVertexDeclarationElementList& Elements);
    static void ClearCacheVertexDeclarations() { VertexDeclarationCache.clear(); }
private:
    static std::unordered_map<FGraphicsPipelineStateInitializer, FRHIPipelineStateRef> GraphicsPipelineCache;
    static std::unordered_map<RHIComputeShader*, FRHIPipelineStateRef> ComputePipelineCache;
    static std::unordered_map<uint32, FRHIVertexDeclarationRef> VertexDeclarationCache;
};

}