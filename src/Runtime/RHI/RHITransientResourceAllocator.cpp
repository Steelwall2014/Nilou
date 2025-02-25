#include "RHITransientResourceAllocator.h"
#include "RHIResources.h"
#include "DynamicRHI.h"

namespace nilou {

// Steelwall2014: This is a simplified implementation of IRHITransientResourceAllocator.
// In Unreal Engine, an implementation of IRHITransientResourceAllocator requires memory aliasing support on different RHI backends
// which is kind of complicated. So I just implement a simple version here, and it's basically an object pool.
class RHITransientResourceAllocator : public IRHITransientResourceAllocator
{
public:
	virtual ~RHITransientResourceAllocator()
    {

    }

	// Supports transient allocations of given resource type
	virtual bool SupportsResourceType(ERHITransientResourceType InType) const;

	// Allocates a new transient resource with memory backed by the transient allocator.
	virtual FRHITransientTexture* CreateTexture(const FRHITextureCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex);
	virtual FRHITransientBuffer* CreateBuffer(const FRHIBufferCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex);

	// Deallocates the underlying memory for use by a future resource creation call.
	virtual void DeallocateMemory(FRHITransientTexture* InTexture, uint32 InPassIndex);
	virtual void DeallocateMemory(FRHITransientBuffer* InBuffer, uint32 InPassIndex);

	// Flushes any pending allocations prior to rendering. Optionally emits stats if OutStats is valid.
	virtual void Flush(RHICommandList& RHICmdList, FRHITransientAllocationStats* OutStats = nullptr);

	// Releases this instance of the transient allocator. Invalidates any outstanding transient resources.
	virtual void Release(RHICommandList& RHICmdList) { delete this; }

private:
    std::unordered_map<FRHITextureCreateInfo, std::vector<FRHITransientTexture*>> TexturePool;
    std::unordered_map<FRHIBufferCreateInfo, std::vector<FRHITransientBuffer*>> BufferPool;

	std::unordered_set<FRHITransientTexture*> AllocatedTextures;
	std::unordered_set<FRHITransientBuffer*> AllocatedBuffers;
};

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