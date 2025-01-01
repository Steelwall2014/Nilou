#pragma once
#include "RenderGraph.h"
#include "RenderGraphResources.h"

namespace nilou {

class FRDGBufferPool : public FRenderResource
{
public:
	FRDGBufferPool() = default;

	/** Call once per frame to trim elements from the pool. */
	void TickPoolElements();

	FRDGPooledBufferRef FindFreeBuffer(const RDGBufferDesc& Desc, const std::string& InDebugName, ERDGPooledBufferAlignment Alignment = ERDGPooledBufferAlignment::Page);

private:
	void ReleaseRHI() override;

	mutable std::recursive_mutex Mutex;

	/** Elements can be 0, we compact the buffer later. */
	std::vector<FRDGPooledBufferRef> AllocatedBuffers;
	std::vector<uint32> AllocatedBufferHashes;

	uint32 FrameCounter = 0;

	friend class RenderGraph;
};

/** The global buffers for easy shading. */
extern TGlobalResource<FRDGBufferPool> GRenderGraphBufferPool;


// Steewall2014: In UE5, it's called "FRenderTargetPool". I guess it's for backward compatibility.
// But we just call it "FRDGTexturePool" here to be symmetrical with "FRDGBufferPool".
/**
 * Encapsulates the render targets pools that allows easy sharing (mostly used on the render thread side)
 */
class FRDGTexturePool : public FRenderResource
{
public:
	FRDGTexturePool() = default;

	FRDGPooledTextureRef FindFreeElement(RDGTextureDesc Desc, const std::string& Name);

	/** Only to get statistics on usage and free elements. Normally only called in renderthread or if FlushRenderingCommands was called() */
	void GetStats(uint32& OutWholeCount, uint32& OutWholePoolInKB, uint32& OutUsedInKB) const;
	/**
	 * Can release RT, should be called once per frame.
	 * call from RenderThread only
	 */
	void TickPoolElements();
	/** Free renderer resources */
	void ReleaseRHI();

	/** Allows to remove a resource so it cannot be shared and gets released immediately instead a/some frame[s] later. */
	void FreeUnusedResource(FRDGPooledTextureRef& In);

	/** Good to call between levels or before memory intense operations. */
	void FreeUnusedResources();

	uint32 GetElementCount() const { return PooledRenderTargets.size(); }

	// @return -1 if not found
	int32 FindIndex(FRDGPooledTexture* In) const;

private:
	void FreeElementAtIndex(int32 Index);

	mutable std::recursive_mutex Mutex;

	/** Elements can be 0, we compact the buffer later. */
	std::vector<uint32> PooledRenderTargetHashes;
	std::vector<FRDGPooledTextureRef> PooledRenderTargets;
	std::vector<FRDGPooledTextureRef> DeferredDeleteArray;

	// redundant, can always be computed with GetStats(), to debug "out of memory" situations and used for r.RenderTargetPoolMin
	uint32 AllocationLevelInKB = 0;

	friend RenderGraph;
};

/** The global render targets for easy shading. */
extern TGlobalResource<FRDGTexturePool> GRenderGraphTexturePool;

class FRDGTransientResourceAllocator : public FRenderResource
{
public:
	IRHITransientResourceAllocator* Get() { return Allocator; }

private:
    virtual void InitRHI(RenderGraph&) override;
    virtual void ReleaseRHI() override;

	IRHITransientResourceAllocator* Allocator = nullptr;
};

extern TGlobalResource<FRDGTransientResourceAllocator, FRenderResource::EInitPhase::Pre> GRDGTransientResourceAllocator;

}
