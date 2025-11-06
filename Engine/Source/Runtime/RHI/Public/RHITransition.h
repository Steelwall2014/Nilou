#pragma once
#include "RHISubresource.h"

namespace nilou {

class RHIMemoryBarrier
{
public:
	ERHIAccess SrcAccess = ERHIAccess::None;
	ERHIAccess DstAccess = ERHIAccess::None;
	EPipelineStageFlags SrcStage = EPipelineStageFlags::None;
	EPipelineStageFlags DstStage = EPipelineStageFlags::None;

	friend class RHICommandList;
};

class RHIBufferMemoryBarrier
{
public:
	RHIBufferMemoryBarrier(RHIBuffer* InBuffer, ERHIAccess InSrcAccess, ERHIAccess InDstAccess, EPipelineStageFlags InSrcStage, EPipelineStageFlags InDstStage, uint64 InOffset, uint64 InSize)
		: Buffer(InBuffer)
		, SrcAccess(InSrcAccess)
		, DstAccess(InDstAccess)
		, SrcStage(InSrcStage)
		, DstStage(InDstStage)
		, Offset(InOffset)
		, Size(InSize)
	{
		Ncheck(Buffer);
	}

	RHIBuffer* Buffer;
	ERHIAccess SrcAccess;
	ERHIAccess DstAccess;
	EPipelineStageFlags SrcStage;
	EPipelineStageFlags DstStage;
	uint64 Offset = 0;
	uint64 Size = 0;

	friend class RHICommandList;
};

class RHIImageMemoryBarrier
{
public:
	RHIImageMemoryBarrier(RHITexture* InTexture, ERHIAccess InSrcAccess, ERHIAccess InDstAccess, EPipelineStageFlags InSrcStage, EPipelineStageFlags InDstStage, ETextureLayout InOldLayout, ETextureLayout InNewLayout, RHITextureSubresource InSubresource)
		: Texture(InTexture)
		, SrcAccess(InSrcAccess)
		, DstAccess(InDstAccess)
		, SrcStage(InSrcStage)
		, DstStage(InDstStage)
		, OldLayout(InOldLayout)
		, NewLayout(InNewLayout)
		, Subresource(InSubresource)
	{
		Ncheck(Texture);
	}

	RHITexture* Texture;
	ERHIAccess SrcAccess;
	ERHIAccess DstAccess;
	EPipelineStageFlags SrcStage;
	EPipelineStageFlags DstStage;
	ETextureLayout OldLayout;
	ETextureLayout NewLayout;
	RHITextureSubresource Subresource;

	friend class RHICommandList;
};

class RHISemaphore : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
{

};
using RHISemaphoreRef = TRefCountPtr<RHISemaphore>;

struct FRHITransientAliasingOverlap
{
	union
	{
		class RHIResource* Resource = nullptr;
		class RHITexture* Texture;
		class RHIBuffer* Buffer;
	};

	enum class EType : uint8
	{
		Texture,
		Buffer
	} Type = EType::Texture;

	FRHITransientAliasingOverlap() = default;

	FRHITransientAliasingOverlap(RHIResource* InResource, EType InType)
		: Resource(InResource)
		, Type(InType)
	{}

	FRHITransientAliasingOverlap(RHITexture* InTexture)
		: Texture(InTexture)
		, Type(EType::Texture)
	{}

	FRHITransientAliasingOverlap(RHIBuffer* InBuffer)
		: Buffer(InBuffer)
		, Type(EType::Buffer)
	{}

	bool IsTexture() const
	{
		return Type == EType::Texture;
	}

	bool IsBuffer() const
	{
		return Type == EType::Buffer;
	}

	bool operator == (const FRHITransientAliasingOverlap& Other) const
	{
		return Resource == Other.Resource;
	}

	inline bool operator != (const FRHITransientAliasingOverlap& RHS) const
	{
		return !(*this == RHS);
	}
};

}