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

FRHIVertexDeclaration* FPipelineStateCache::GetOrCreateVertexDeclaration(const FVertexDeclarationElementList& Elements)
{
	uint32 Key = FCrc::MemCrc32(Elements.data(), Elements.size() * sizeof(FVertexElement));
    auto Found = VertexDeclarationCache.find(Key);
    if (Found != VertexDeclarationCache.end())
    {
        return Found->second.get();
    }
    
	FRHIVertexDeclarationRef NewDeclaration = FDynamicRHI::GetDynamicRHI()->RHICreateVertexDeclaration(Elements);
    VertexDeclarationCache.insert({Key, NewDeclaration});
    return NewDeclaration.get();
}

std::unordered_map<FGraphicsPipelineStateInitializer, FRHIGraphicsPipelineStateRef> 
FPipelineStateCache::GraphicsPipelineCache{};

std::unordered_map<uint32, FRHIVertexDeclarationRef> 
FPipelineStateCache::VertexDeclarationCache{};

}