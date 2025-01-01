#pragma once
#include <string>

#include "Platform.h"
#include "RHIResources.h"
#include "Templates/TypeTraits.h"
#include "RenderGraphDefinitions.h"
#include "RenderGraphTextureSubresource.h"

namespace nilou {

/** Used for tracking the state of an individual subresource during execution. */
class RDGSubresourceState
{
public:

	/** The last used access on the pass. */
	ERHIAccess Access = ERHIAccess::None;

	/** The last pass in this state. */
	FRDGPassHandle Pass;

};

class FRDGPooledBuffer : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
{
public:
    friend class FRDGBufferPool;
	FRDGPooledBuffer(RHIBufferRef InBuffer, const RDGBufferDesc& InDesc, uint32 InNumAllocatedElements, const std::string& InName)
		: Desc(InDesc)
		, Buffer(InBuffer)
		, Name(InName)
		, NumAllocatedElements(InNumAllocatedElements)
	{

	}

    RHIBuffer* GetRHI() const { return Buffer.GetReference(); }

	FORCEINLINE uint32 GetSize() const
	{
		return Desc.GetSize();
	}

private:
    const RDGBufferDesc Desc;
    RHIBufferRef Buffer;

	RDGBufferDesc GetAlignedDesc() const
	{
		RDGBufferDesc AlignedDesc = Desc;
		AlignedDesc.NumElements = NumAllocatedElements;
		return AlignedDesc;
	}

	// Used internally by FRDGBuilder::QueueCommitReservedBuffer(),
	// which is expected to be the only way to resize physical memory for FRDGPooledBuffer
	void SetCommittedSize(uint64 InCommittedSizeInBytes)
	{
		if (InCommittedSizeInBytes == UINT64_MAX)
		{
			InCommittedSizeInBytes = GetSize();
		}

		Ncheckf(EnumHasAllFlags(Desc.Usage, EBufferUsageFlags::ReservedResource), "CommitReservedResource() may only be used on reserved buffers");
		Ncheckf(InCommittedSizeInBytes <= GetSize(), "Attempting to commit more memory than was reserved for this buffer during creation");

		CommittedSizeInBytes = InCommittedSizeInBytes;
	}

    std::string Name;

	// Size of the GPU physical memory committed to a reserved buffer.
	// May be UINT64_MAX for regular (non-reserved) buffers or when the entire resource is committed.
	uint64 CommittedSizeInBytes = UINT64_MAX;

	const uint32 NumAllocatedElements;
	uint32 LastUsedFrame = 0;
};
using FRDGPooledBufferRef = TRefCountPtr<FRDGPooledBuffer>;

class FRDGPooledTexture : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
{
public:
    friend class FRDGTexturePool;
    FRDGPooledTexture(RHITextureRef InTexture, const RDGTextureDesc& InDesc, const std::string& InName)
        : Desc(InDesc)
        , Texture(InTexture)
        , Name(InName)
    { }

    uint32 ComputeMemorySize() const;

    RHITexture* GetRHI() const { return Texture.GetReference(); }
    
private:
    const RDGTextureDesc Desc;
    RHITextureRef Texture;

    std::string Name;
    uint32 UnusedForNFrames = 0;
};
using FRDGPooledTextureRef = TRefCountPtr<FRDGPooledTexture>;

/** Used for tracking pass producer / consumer edges in the graph for culling and pipe fencing. */
struct FRDGProducerState
{
	FRDGPass* Pass = nullptr;
	ERHIAccess Access = ERHIAccess::None;
};

class RDGResource
{
public:
    friend class FRDGPass;
    friend class RenderGraph;
    
    RDGResource(std::string InName, ERDGResourceType InResourceType) : Name(InName), Type(InResourceType) { }
	RDGResource(const RDGResource&) = delete;

	bool IsCulled() const
	{
		return ReferenceCount == 0;
	}

	bool IsCullRoot() const
	{
		return bExternal || bExtracted;
	}

    std::string Name;
    
    bool bIsPersistent = false;

    ERDGResourceType Type;

protected:

    TRefCountPtr<RHIResource> ResourceRHI = nullptr;

	/** Whether this is an externally registered resource. */
	bool bExternal = false;

	/** Whether this is an extracted resource. */
	bool bExtracted = false;

	/** Whether any sub-resource has been used for write by a pass. */
	bool bProduced = false;

	/** Whether this resource is allocated through the transient resource allocator. */
	bool bTransient = false;

	/** If false, the resource needs to be collected. */
	bool bCollectForAllocate = true;

	FRDGPassHandle FirstPass;
	FRDGPassHandle LastPass;
	FRDGPassHandle MinAcquirePass;
	FRDGPassHandle MinDiscardPass;

	/** Number of references in passes and deferred queries. */
	uint16 ReferenceCount = 0;

	/** Scratch index allocated for the resource in the pass being setup. */
	uint16 PassStateIndex = 0;

	static const uint16 DeallocatedReferenceCount = ~0;

};

/************ Texture *************/
using RDGTextureDesc = RHITextureDesc;
class RDGTexture : public RDGResource
{
public:
    friend class RenderGraph;
    
    RDGTexture(std::string InName, const RDGTextureDesc& InDesc);

    RHITexture* GetRHI() const { return static_cast<RHITexture*>(ResourceRHI.GetReference()); }

    class RDGTextureView* GetDefaultView() const { return DefaultView; }

	FRDGTextureSubresourceLayout GetSubresourceLayout() const
	{
		return Layout;
	}

	FRDGTextureSubresourceRange GetSubresourceRange() const
	{
		return WholeRange;
	}

	uint16 GetSubresourceCount() const
	{
		return SubresourceCount;
	}

	FRDGTextureSubresource GetSubresource(uint32 SubresourceIndex) const
	{
		return Layout.GetSubresource(SubresourceIndex);
	}

    const RDGTextureDesc Desc;

private:

    RDGTextureView* DefaultView;
    
	/** The layout used to facilitate subresource transitions. */
	FRDGTextureSubresourceLayout Layout;
	FRDGTextureSubresourceRange  WholeRange;
	const uint16 SubresourceCount;

	/** Tracks pass producers for each subresource as the graph is built. */
	std::vector<FRDGProducerState> LastProducers;

    // Steelwall2014: not null if the texture is created from RenderGraph::CreateTexture
    class FRHITransientTexture* TransientTexture;
    // Steelwall2014: not null if the texture is created from RenderGraph::CreateExternalTexture
    FRDGPooledTextureRef PooledTexture;

    std::vector<RDGSubresourceState> State;

};
using RDGTextureRef = std::shared_ptr<RDGTexture>;

struct RDGTextureViewDesc
{
    EPixelFormat Format; 
    uint32 BaseMipLevel;
    uint32 LevelCount;
    uint32 BaseArrayLayer;
    uint32 LayerCount;
    ETextureDimension ViewType;
    RDGTexture* Texture;

    RDGTextureViewDesc() = default;

    RDGTextureViewDesc(RDGTexture* InTexture)
    {
        Texture = InTexture;
        Format = InTexture->Desc.Format;
        BaseMipLevel = 0;
        LevelCount = InTexture->Desc.NumMips;
        BaseArrayLayer = 0;
        LayerCount = InTexture->Desc.ArraySize;
        ViewType = InTexture->Desc.TextureType;
    }

    static RDGTextureViewDesc Create(RDGTexture* InTexture)
    {
        return RDGTextureViewDesc(InTexture);
    }

    static RDGTextureViewDesc CreateForMipLevel(RDGTexture* InTexture, uint32 MipLevel)
    {
        RDGTextureViewDesc Desc = RDGTextureViewDesc::Create(InTexture);
        Desc.BaseMipLevel = MipLevel;
        Desc.LevelCount = 1;
        return Desc;
    }

    bool operator==(const RDGTextureViewDesc& Other) const = default;
};
class RDGTextureView : public RDGResource
{
public:
    RDGTextureView(std::string InName, const RDGTextureViewDesc& InDesc) 
        : RDGResource(InName, ERDGResourceType::TextureView)
        , Desc(InDesc)
    { }

    const RDGTextureViewDesc Desc;

    RDGTexture* GetParent() const { return Desc.Texture; }

    const FRDGTextureSubresourceRange& GetSubresourceRange() const { return SubresourceRange; }

    FRDGTextureSubresourceRange SubresourceRange;
};
using RDGTextureViewRef = std::shared_ptr<RDGTextureView>;

/************ Buffer *************/
class RDGBuffer : public RDGResource
{
public:
    RDGBuffer(std::string InName, const RDGBufferDesc& InDesc) 
        : RDGResource(InName, ERDGResourceType::Buffer)
        , Desc(InDesc) 
    { 
        Data = std::make_unique<uint8[]>(InDesc.GetSize());
        AllocatedSize = InDesc.GetSize();
    }
    RHIBuffer* GetRHI() const { return static_cast<RHIBuffer*>(ResourceRHI.GetReference()); }

    void SetData(const void* InData, uint32 Size, uint32 Offset)
    {
        Ncheckf(Offset+Size <= AllocatedSize, "Data size is too large, expected %d, got %d", AllocatedSize, Offset+Size);
        if (Data)
        {
            if (memcmp(Data.get(), InData, Size) != 0)
            {
                memcpy(Data.get()+Offset, InData, Size);
                bDirty = true;
            }
        }
    }

    template<typename T>
    const T* GetData() const
    {
        Ncheckf(sizeof(T) <= AllocatedSize, "Data size is too large");
        return reinterpret_cast<T*>(Data.get());
    }

    void Flush();

    const RDGBufferDesc Desc;
    std::unique_ptr<uint8[]> Data = nullptr;
    uint32 AllocatedSize = 0;
    bool bDirty = false;

    // Steelwall2014: not null if the buffer is created from RenderGraph::CreateBuffer
    class FRHITransientBuffer* TransientBuffer;
    // Steelwall2014: not null if the buffer is created from RenderGraph::CreateExternalBuffer
    FRDGPooledBufferRef PooledBuffer;

    RDGSubresourceState State;
};
using RDGBufferRef = std::shared_ptr<RDGBuffer>;

template <typename T>
class TRDGUniformBuffer : public RDGBuffer
{
public:
    using DataType = T;
    template <auto MemberPointer, typename InFieldType = typename MemberPointerTraits<decltype(MemberPointer)>::Value>
    void SetData(const InFieldType& Value)
    {
        using InDataType = typename MemberPointerTraits<decltype(MemberPointer)>::Object;
        static_assert(std::is_member_pointer_v<decltype(MemberPointer)>);
        static_assert(std::is_trivially_copyable_v<InDataType>);
        static_assert(std::is_trivially_copyable_v<InFieldType>);
        static_assert(std::is_same_v<InDataType, T>);
        if (Data)
        {
            DataType& data = *reinterpret_cast<DataType*>(Data.get());
            if (memcmp(&(data.*MemberPointer), &Value, sizeof(InFieldType)) != 0)
            {
                data.*MemberPointer = Value;
                bDirty = true;
            }
        }
    }
};
template <typename T>
using TRDGUniformBufferRef = std::shared_ptr<TRDGUniformBuffer<T>>;

class RDGFramebuffer
{
public:

    void SetAttachment(EFramebufferAttachment Attachment, RDGTextureView* Texture);

    const RHIRenderTargetLayout& GetRenderTargetLayout() const { return RTLayout; }

private:

    RHIRenderTargetLayout RTLayout;

    std::unordered_map<EFramebufferAttachment, RDGTextureView*> Attachments;
};

}
