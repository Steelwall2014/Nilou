#include "PipelineStateCache.h"
#include "Common/Crc.h"
#include "DynamicRHI.h"

namespace nilou {

void FPipelineStateCache::CacheGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer, RHIGraphicsPipelineStateRef CacheState)
{
    GraphicsPipelineCache[Initializer] = CacheState;
}

RHIGraphicsPipelineState* FPipelineStateCache::FindCachedGraphicsPSO(const FGraphicsPipelineStateInitializer& Initializer)
{
    if (!GraphicsPipelineCache.contains(Initializer))
        return nullptr;
    return GraphicsPipelineCache[Initializer].GetReference();
}

void FPipelineStateCache::CacheComputePSO(RHIComputeShader* ComputeShader, RHIComputePipelineStateRef CacheState)
{
    ComputePipelineCache[ComputeShader] = CacheState;
}

RHIComputePipelineState* FPipelineStateCache::FindCachedComputePSO(RHIComputeShader* ComputeShader)
{
    if (!ComputePipelineCache.contains(ComputeShader))
        return nullptr;
    return ComputePipelineCache[ComputeShader].GetReference();
}

void FPipelineStateCache::CacheVertexDeclaration(const FVertexDeclarationElementList& ElementList, FRHIVertexDeclarationRef VertexDeclaration)
{
    uint32 Key = FCrc::MemCrc32(ElementList.data(), ElementList.size() * sizeof(FVertexElement));
    VertexDeclarationCache[Key] = VertexDeclaration;
}

FRHIVertexDeclaration* FPipelineStateCache::FindVertexDeclaration(const FVertexDeclarationElementList& Elements)
{
    uint32 Key = FCrc::MemCrc32(Elements.data(), Elements.size() * sizeof(FVertexElement));
    if (!VertexDeclarationCache.contains(Key))
        return nullptr;
    return VertexDeclarationCache[Key].GetReference();
}

std::unordered_map<FGraphicsPipelineStateInitializer, RHIGraphicsPipelineStateRef> 
FPipelineStateCache::GraphicsPipelineCache{};

std::unordered_map<RHIComputeShader*, RHIComputePipelineStateRef> 
FPipelineStateCache::ComputePipelineCache{};

std::unordered_map<uint32, FRHIVertexDeclarationRef> 
FPipelineStateCache::VertexDeclarationCache{};

}