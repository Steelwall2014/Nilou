#include "RenderGraphResourcePool.h"
#include "Templates/AlignmentTemplates.h"
#include "Templates/TypeHash.h"
#include "ProfilingDebugging/ContersTrace.h"

namespace nilou {

static uint32 ComputeSizeInKB(FRDGPooledTexture& Element)
{
	return (Element.ComputeMemorySize() + 1023) / 1024;
}

FRDGPooledBufferRef FRDGBufferPool::FindFreeBuffer(const RDGBufferDesc& Desc, const std::string& InDebugName, ERDGPooledBufferAlignment Alignment)
{
	const uint64 BufferPageSize = 64 * 1024;

	RDGBufferDesc AlignedDesc = Desc;

	switch (Alignment)
	{
	case ERDGPooledBufferAlignment::PowerOfTwo:
		AlignedDesc.NumElements = FMath::RoundUpToPowerOfTwo(AlignedDesc.BytesPerElement * AlignedDesc.NumElements) / AlignedDesc.BytesPerElement;
		// Fall through to align up to page size for small buffers; helps with reuse.

	case ERDGPooledBufferAlignment::Page:
		AlignedDesc.NumElements = Align(AlignedDesc.BytesPerElement * AlignedDesc.NumElements, BufferPageSize) / AlignedDesc.BytesPerElement;
	}

    if (AlignedDesc.NumElements >= Desc.NumElements)
    {
        NILOU_LOG(Warning, "Alignment caused buffer size overflow for buffer '{}' (AlignedDesc.NumElements: {} < Desc.NumElements: {})", InDebugName, AlignedDesc.NumElements, Desc.NumElements);
		// Use the unaligned desc since we apparently overflowed when rounding up.
		AlignedDesc = Desc;
    }

	const uint32 BufferHash = GetTypeHash(AlignedDesc);
    
    std::lock_guard<std::recursive_mutex> Lock(Mutex);

	// First find if available.
	for (int32 Index = 0; Index < AllocatedBufferHashes.size(); ++Index)
	{
		if (AllocatedBufferHashes[Index] != BufferHash)
		{
			continue;
		}

		const auto& PooledBuffer = AllocatedBuffers[Index];

		// Still being used outside the pool.
		if (PooledBuffer->GetSize() > 1)
		{
			continue;
		}

		Ncheck(PooledBuffer->GetAlignedDesc() == AlignedDesc);

		PooledBuffer->LastUsedFrame = FrameCounter;
		PooledBuffer->Name = InDebugName;

		// We need the external-facing desc to match what the user requested.
		const_cast<RDGBufferDesc&>(PooledBuffer->Desc).NumElements = Desc.NumElements;

		return PooledBuffer;
	}

	// Allocate new one
	{
		const uint32 NumBytes = AlignedDesc.GetSize();

		RHIBufferRef BufferRHI = RHICreateBuffer(RHIBufferDesc(NumBytes, AlignedDesc.GetStride()), InDebugName);

		FRDGPooledBufferRef PooledBuffer = new FRDGPooledBuffer(std::move(BufferRHI), Desc, AlignedDesc.NumElements, InDebugName);
		AllocatedBuffers.push_back(PooledBuffer);
		AllocatedBufferHashes.push_back(BufferHash);
		Ncheck(PooledBuffer->GetRefCount() == 2);

		PooledBuffer->LastUsedFrame = FrameCounter;

		if (EnumHasAllFlags(Desc.Usage, EBufferUsageFlags::ReservedResource))
		{
			PooledBuffer->CommittedSizeInBytes = 0;
		}

		return PooledBuffer;
	}

}

FRDGPooledTextureRef FRDGTexturePool::FindFreeElement(RDGTextureDesc Desc, const std::string& Name)
{
	FRDGPooledTextureRef Found = nullptr;
	uint32 FoundIndex = -1;

	// FastVRAM is no longer supported by the render target pool.
	EnumRemoveFlags(Desc.Flags, ETextureCreateFlags::FastVRAM | ETextureCreateFlags::FastVRAMPartialAlloc);

	// We always want SRV access
	Desc.Flags |= TexCreate_ShaderResource;

	const uint32 DescHash = GetTypeHash(Desc);

	std::lock_guard<std::recursive_mutex> Lock(Mutex);

	for (uint32 Index = 0, Num = (uint32)PooledRenderTargets.size(); Index < Num; ++Index)
	{
		if (PooledRenderTargetHashes[Index] == DescHash)
		{
			FRDGPooledTextureRef& Element = PooledRenderTargets[Index];

			if (Element->GetRefCount() == 1)
			{
				Found = Element;
				FoundIndex = Index;
				break;
			}
		}
	}

	if (!Found)
	{
        RHITextureRef TextureRHI = RHICreateTexture(Desc, Name);

		Found = new FRDGPooledTexture(
			TextureRHI,
			Desc, Name);

		PooledRenderTargets.push_back(Found);
		PooledRenderTargetHashes.push_back(DescHash);

		AllocationLevelInKB += ComputeSizeInKB(*Found);
		TRACE_COUNTER_ADD(RenderTargetPoolCount, 1);
		TRACE_COUNTER_SET(RenderTargetPoolSize, (int64)AllocationLevelInKB * 1024);

		FoundIndex = PooledRenderTargets.size() - 1;
	}

	Found->UnusedForNFrames = 0;

    return Found;
}

TGlobalResource<FRDGTransientResourceAllocator, FRenderResource::EInitPhase::Pre> GRDGTransientResourceAllocator;

}
