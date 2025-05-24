#include "RHITransientResourceAllocator.h"
#include "RHIResources.h"
#include "DynamicRHI.h"

namespace nilou {

bool RHITransientResourceAllocator::SupportsResourceType(ERHITransientResourceType InType) const
{
    return InType == ERHITransientResourceType::Texture || InType == ERHITransientResourceType::Buffer;
}

FRHITransientTexture* RHITransientResourceAllocator::CreateTexture(const FRHITextureCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex)
{
	std::vector<FRHITransientTexture*>& Pool = TexturePool[InCreateInfo];
	if (Pool.size() == 0)
	{
		RHITexture* TextureRHI = RHICreateTexture(InCreateInfo, InDebugName);
		FRHITransientTexture* Texture = new FRHITransientTexture(TextureRHI, 0, 0, 0, ERHITransientAllocationType::Heap, InCreateInfo);
		AllocatedTextures.insert(Texture);
		return Texture;
	}
	else 
	{
		FRHITransientTexture* Texture = Pool.back(); Pool.pop_back();
		AllocatedTextures.insert(Texture);
		return Texture;
	}
}

FRHITransientBuffer* RHITransientResourceAllocator::CreateBuffer(const FRHIBufferCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex)
{
	std::vector<FRHITransientBuffer*>& Pool = BufferPool[InCreateInfo];
	if (Pool.size() == 0)
	{
		RHIBuffer* BufferRHI = RHICreateBuffer(InCreateInfo, InDebugName);
		FRHITransientBuffer* Buffer = new FRHITransientBuffer(BufferRHI, 0, 0, 0, ERHITransientAllocationType::Heap, InCreateInfo);
		AllocatedBuffers.insert(Buffer);
		return Buffer;
	}
	else
	{
		FRHITransientBuffer* Buffer = Pool.back(); Pool.pop_back();
		AllocatedBuffers.insert(Buffer);
		return Buffer;
	}
}

void RHITransientResourceAllocator::DeallocateMemory(FRHITransientTexture* InTexture, uint32 InPassIndex)
{
	Ncheck(AllocatedTextures.find(InTexture) != AllocatedTextures.end());
	AllocatedTextures.erase(InTexture);
	const FRHITextureCreateInfo& CreateInfo = InTexture->GetCreateInfo();
	TexturePool[CreateInfo].push_back(InTexture);
}

void RHITransientResourceAllocator::DeallocateMemory(FRHITransientBuffer* InBuffer, uint32 InPassIndex)
{
	Ncheck(AllocatedBuffers.find(InBuffer) != AllocatedBuffers.end());
	AllocatedBuffers.erase(InBuffer);
	const FRHIBufferCreateInfo& CreateInfo = InBuffer->GetCreateInfo();
	BufferPool[CreateInfo].push_back(InBuffer);
}

void RHITransientResourceAllocator::Flush(RHICommandList& RHICmdList, FRHITransientAllocationStats* OutStats)
{
	
}

}