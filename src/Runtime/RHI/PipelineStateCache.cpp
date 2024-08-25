#include "PipelineStateCache.h"
#include "Common/Crc.h"
#include "DynamicRHI.h"

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

void FPipelineStateCache::CacheComputePSO(RHIComputeShader* ComputeShader, FRHIComputePipelineStateRef CacheState)
{
    ComputePipelineCache[ComputeShader] = CacheState;
}

FRHIComputePipelineState* FPipelineStateCache::FindCachedComputePSO(RHIComputeShader* ComputeShader)
{
    if (!ComputePipelineCache.contains(ComputeShader))
        return nullptr;
    return ComputePipelineCache[ComputeShader].get();
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
    return VertexDeclarationCache[Key].get();
}

std::unordered_map<FGraphicsPipelineStateInitializer, FRHIGraphicsPipelineStateRef> 
FPipelineStateCache::GraphicsPipelineCache{};

std::unordered_map<RHIComputeShader*, FRHIComputePipelineStateRef> 
FPipelineStateCache::ComputePipelineCache{};

std::unordered_map<uint32, FRHIVertexDeclarationRef> 
FPipelineStateCache::VertexDeclarationCache{};

}