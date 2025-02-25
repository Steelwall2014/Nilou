#pragma once

#include "RHI.h"
#include "RHIResources.h"
#include "Templates/Interval.h"
#include "RHITransition.h"

// Steelwall2014: This file is copy-pasted from Unreal Engine for educational purposes.
// I want to learn how to implement a memory allocator.

namespace nilou {

class RHICommandList;
class FRHITransientHeap;
class FRHITransientPagePool;

struct FRHITransientPageSpan
{
	// Offset of the span in the page pool in pages. 
	uint16 Offset = 0;

	// Number of pages in the span.
	uint16 Count = 0;
};

/** Represents an allocation from a transient page pool. */
struct FRHITransientPagePoolAllocation
{
	bool IsValid() const { return Pool != nullptr; }

	// The transient page pool which made the allocation.
	FRHITransientPagePool* Pool = nullptr;

	// A unique hash identifying this allocation to the allocator implementation.
	uint64 Hash = 0;

	// The index identifying the allocation to the page pool.
	uint16 SpanIndex = 0;

	// Offsets into the array of spans for the allocator implementation.
	uint16 SpanOffsetMin = 0;
	uint16 SpanOffsetMax = 0;
};

/** Represents a full set of page allocations from multiple page pools. */
struct FRHITransientPageAllocation
{
	// The list of allocations by pool.
	std::vector<FRHITransientPagePoolAllocation> PoolAllocations;

	// The full list of spans indexed by each allocation.
	std::vector<FRHITransientPageSpan> Spans;
};

/** Represents an allocation from the transient heap. */
struct FRHITransientHeapAllocation
{
	bool IsValid() const { return Size != 0; }

	// Transient heap which made the allocation.
	FRHITransientHeap* Heap = nullptr;

	// Size of the allocation made from the allocator (aligned).
	uint64 Size = 0;

	// Offset in the transient heap; front of the heap starts at 0.
	uint64 Offset = 0;

	// Number of bytes of padding were added to the offset.
	uint32 AlignmentPad = 0;
};

enum class ERHITransientResourceType : uint8
{
	Texture,
	Buffer
};

enum class ERHITransientAllocationType : uint8
{
	Heap,
	Page
};

class FRHITransientResource
{
public:
	static const uint32 kInvalidPassIndex = std::numeric_limits<uint32>::max();

	FRHITransientResource(
		RHIResource* InResource,
		uint64 InGpuVirtualAddress,
		uint64 InHash,
		uint64 InSize,
		ERHITransientAllocationType InAllocationType,
		ERHITransientResourceType InResourceType)
		: Resource(InResource)
		, GpuVirtualAddress(InGpuVirtualAddress)
		, Hash(InHash)
		, Size(InSize)
		, AllocationType(InAllocationType)
		, ResourceType(InResourceType)
	{}

	virtual ~FRHITransientResource() = default;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//! Internal Allocator API

	virtual void Acquire(RHICommandList& RHICmdList, const std::string& InName, uint32 InAcquirePassIndex, uint64 InAllocatorCycle)
	{
		Name = InName;
		AcquirePasses = TInterval<uint32>(0, InAcquirePassIndex);
		DiscardPasses = TInterval<uint32>(kInvalidPassIndex, kInvalidPassIndex);
		AcquireCycle = InAllocatorCycle;
		AcquireCount++;
		AliasingOverlaps.clear();
	}

	void Discard(uint32 InDiscardPassIndex)
	{
		DiscardPasses.Min = InDiscardPassIndex;
	}

	void AddAliasingOverlap(FRHITransientResource* InResource)
	{
		AliasingOverlaps.emplace_back(InResource->GetRHI(), InResource->IsTexture() ? FRHITransientAliasingOverlap::EType::Texture : FRHITransientAliasingOverlap::EType::Buffer);

		Ncheck(InResource->DiscardPasses.Min != kInvalidPassIndex);

		InResource->DiscardPasses.Max = std::min(InResource->DiscardPasses.Max,             AcquirePasses.Max);
		            AcquirePasses.Min = std::max(            AcquirePasses.Min, InResource->DiscardPasses.Min);
	}

	FRHITransientHeapAllocation& GetHeapAllocation()
	{
		Ncheck(AllocationType == ERHITransientAllocationType::Heap);
		return HeapAllocation;
	}

	const FRHITransientHeapAllocation& GetHeapAllocation() const
	{
		Ncheck(AllocationType == ERHITransientAllocationType::Heap);
		return HeapAllocation;
	}

	FRHITransientPageAllocation& GetPageAllocation()
	{
		Ncheck(IsPageAllocated());
		return PageAllocation;
	}

	const FRHITransientPageAllocation& GetPageAllocation() const
	{
		Ncheck(IsPageAllocated());
		return PageAllocation;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns the underlying RHI resource.
	RHIResource* GetRHI() const { return Resource.get(); }

	// Returns the gpu virtual address of the transient resource.
	uint64 GetGpuVirtualAddress() const { return GpuVirtualAddress; }

	// Returns the name assigned to the transient resource at allocation time.
	const std::string& GetName() const { return Name; }

	// Returns the hash used to uniquely identify this resource if cached.
	uint64 GetHash() const { return Hash; }

	// Returns the required size in bytes of the resource.
	uint64 GetSize() const { return Size; }

	// Returns the last allocator cycle this resource was acquired.
	uint64 GetAcquireCycle() const { return AcquireCycle; }

	// Returns the number of times Acquire has been called.
	uint32 GetAcquireCount() const { return AcquireCount; }

	// Returns the list of aliasing overlaps used when transitioning the resource.
	const std::vector<FRHITransientAliasingOverlap>& GetAliasingOverlaps() const { return AliasingOverlaps; }

	// Returns the pass index which may end acquiring this resource.
	TInterval<uint32> GetAcquirePasses() const { return AcquirePasses; }

	// Returns the pass index which discarded this resource.
	TInterval<uint32> GetDiscardPasses() const { return DiscardPasses; }

	// Returns whether this resource is still in an acquired state.
	bool IsAcquired() const { return DiscardPasses.Min == kInvalidPassIndex; }

	ERHITransientResourceType GetResourceType() const { return ResourceType; }

	bool IsTexture() const { return ResourceType == ERHITransientResourceType::Texture; }
	bool IsBuffer()  const { return ResourceType == ERHITransientResourceType::Buffer; }

	ERHITransientAllocationType GetAllocationType() const { return AllocationType; }

	bool IsHeapAllocated() const { return AllocationType == ERHITransientAllocationType::Heap; }
	bool IsPageAllocated() const { return AllocationType == ERHITransientAllocationType::Page; }

private:
	// Underlying RHI resource.
	std::shared_ptr<RHIResource> Resource;

	// The Gpu virtual address of the RHI resource.
	uint64 GpuVirtualAddress = 0;

	// The hash used to uniquely identify this resource if cached.
	uint64 Hash;

	// Size of the resource in bytes.
	uint64 Size;

	// Alignment of the resource in bytes.
	uint32 Alignment;

	// Tracks the number of times Acquire has been called.
	uint32 AcquireCount = 0;

	// Cycle count used to deduce age of the resource.
	uint64 AcquireCycle = 0;

	// Debug name of the resource. Updated with each allocation.
	std::string Name;

	FRHITransientHeapAllocation HeapAllocation;
	FRHITransientPageAllocation PageAllocation;

	// List of aliasing resources overlapping with this one.
	std::vector<FRHITransientAliasingOverlap> AliasingOverlaps;

	// Start -> End split pass index intervals for acquire / discard operations.
	TInterval<uint32> AcquirePasses = TInterval<uint32>(0, 0);
	TInterval<uint32> DiscardPasses = TInterval<uint32>(0, 0);

	ERHITransientAllocationType AllocationType;
	ERHITransientResourceType ResourceType;
};

class FRHITransientTexture final : public FRHITransientResource
{
public:
	FRHITransientTexture(
		RHITexture* InTexture,
		uint64 InGpuVirtualAddress,
		uint64 InHash,
		uint64 InSize,
		ERHITransientAllocationType InAllocationType,
		const FRHITextureCreateInfo& InCreateInfo)
		: FRHITransientResource(InTexture, InGpuVirtualAddress, InHash, InSize, InAllocationType, ERHITransientResourceType::Texture)
		, CreateInfo(InCreateInfo)
	{}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//! Internal Allocator API
	void Acquire(RHICommandList& RHICmdList, const std::string& InName, uint32 InAcquirePassIndex, uint64 InInitCycle) override NILOU_NOT_IMPLEMENTED
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns the underlying RHI texture.
	RHITexture* GetRHI() const { return static_cast<RHITexture*>(FRHITransientResource::GetRHI()); }

	// Returns the create info struct used when creating this texture.
	const FRHITextureCreateInfo& GetCreateInfo() const { return CreateInfo; }

	// The create info describing the texture.
	const FRHITextureCreateInfo CreateInfo;
};

class FRHITransientBuffer final : public FRHITransientResource
{
public:
	FRHITransientBuffer(
		RHIBuffer* InBuffer,
		uint64 InGpuVirtualAddress,
		uint64 InHash,
		uint64 InSize,
		ERHITransientAllocationType InAllocationType,
		const FRHIBufferCreateInfo& InCreateInfo)
		: FRHITransientResource(InBuffer, InGpuVirtualAddress, InHash, InSize, InAllocationType, ERHITransientResourceType::Buffer)
		, CreateInfo(InCreateInfo)
	{}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//! Internal Allocator API
	void Acquire(RHICommandList& RHICmdList, const std::string& InName, uint32 InAcquirePassIndex, uint64 InInitCycle) override NILOU_NOT_IMPLEMENTED
	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns the underlying RHI buffer.
	RHIBuffer* GetRHI() const { return static_cast<RHIBuffer*>(FRHITransientResource::GetRHI()); }

	// Returns the create info used when creating this buffer.
	const FRHIBufferCreateInfo& GetCreateInfo() const { return CreateInfo; }

	// The create info describing the texture.
	const FRHIBufferCreateInfo CreateInfo;
};

class FRHITransientAllocationStats
{
public:
	struct FAllocation
	{
		uint64 OffsetMin = 0;
		uint64 OffsetMax = 0;
		uint32 MemoryRangeIndex = 0;
	};

	using FAllocationArray = std::vector<FAllocation>;

	enum class EMemoryRangeFlags
	{
		None = 0,

		// The memory range references platform specific fast RAM.
		FastVRAM = 1 << 0
	};

	struct FMemoryRange
	{
		// Number of bytes available for use in the memory range.
		uint64 Capacity = 0;

		// Number of bytes allocated for use in the memory range.
		uint64 CommitSize = 0;

		// Flags specified for this memory range.
		EMemoryRangeFlags Flags = EMemoryRangeFlags::None;
	};

	std::vector<FMemoryRange> MemoryRanges;
	std::map<const FRHITransientResource*, FAllocationArray> Resources;
};

ENUM_CLASS_FLAGS(FRHITransientAllocationStats::EMemoryRangeFlags);

class IRHITransientResourceAllocator
{
public:
	virtual ~IRHITransientResourceAllocator() = default;

	// Supports transient allocations of given resource type
	virtual bool SupportsResourceType(ERHITransientResourceType InType) const = 0;

	// Allocates a new transient resource with memory backed by the transient allocator.
	virtual FRHITransientTexture* CreateTexture(const FRHITextureCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex) = 0;
	virtual FRHITransientBuffer* CreateBuffer(const FRHIBufferCreateInfo& InCreateInfo, const std::string& InDebugName, uint32 InPassIndex) = 0;

	// Deallocates the underlying memory for use by a future resource creation call.
	virtual void DeallocateMemory(FRHITransientTexture* InTexture, uint32 InPassIndex) = 0;
	virtual void DeallocateMemory(FRHITransientBuffer* InBuffer, uint32 InPassIndex) = 0;

	// Flushes any pending allocations prior to rendering. Optionally emits stats if OutStats is valid.
	virtual void Flush(RHICommandList& RHICmdList, FRHITransientAllocationStats* OutStats = nullptr) = 0;

	// Releases this instance of the transient allocator. Invalidates any outstanding transient resources.
	virtual void Release(RHICommandList& RHICmdList) { delete this; }
};

}