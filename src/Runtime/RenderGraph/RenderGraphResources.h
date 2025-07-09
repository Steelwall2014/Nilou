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
	FRDGPassHandle Pass = NullPassHandle;

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

    std::string Name;

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

    bool IsFree() const 
    { 
        uint32 RefCount = GetRefCount();
        Ncheck(RefCount >= 1);

        return RefCount == 1; 
    }
    
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

// TODO: EResourceLifetime
class RDGResource : public TRefCountedObject<ERefCountingMode::NotThreadSafe>
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
		return bExternal/* || bExtracted*/;
	}

    std::string Name;

    ERDGResourceType Type;

protected:

    TRefCountPtr<RHIResource> ResourceRHI = nullptr;

	/** Whether this is an externally registered resource. */
	bool bExternal = false;

	/** Whether this is an extracted resource. */
	// bool bExtracted = false;

	/** Whether any sub-resource has been used for write by a pass. */
	// bool bProduced = false;

	/** Whether this resource is allocated through the transient resource allocator. */
	bool bTransient = false;

	/** If false, the resource needs to be collected. */
	bool bCollectForAllocate = true;

	FRDGPassHandle FirstPass = NullPassHandle;
	FRDGPassHandle LastPass = NullPassHandle;
	FRDGPassHandle MinAcquirePass = NullPassHandle;
	FRDGPassHandle MinDiscardPass = NullPassHandle;

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

    class RDGTextureView* GetDefaultView() const 
    { 
        if (!bTransient)
        {
            return PooledDefaultView.GetReference();
        }
        else 
        {
            return TransientDefaultView;
        }
    }

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
    
    class RDGTextureView* TransientDefaultView = nullptr;
    TRefCountPtr<RDGTextureView> PooledDefaultView = nullptr;

	/** The layout used to facilitate subresource transitions. */
	FRDGTextureSubresourceLayout Layout;
	FRDGTextureSubresourceRange  WholeRange;
	const uint16 SubresourceCount;

	/** Tracks pass producers for each subresource as the graph is built. */
	std::vector<FRDGProducerState> LastProducers;

    // Steelwall2014: not null if the texture is created from RenderGraph::CreateTexture
    class FRHITransientTexture* TransientTexture = nullptr;

    // Steelwall2014: not null if the texture is created from RenderGraph::CreateExternalTexture
    FRDGPooledTextureRef PooledTexture = nullptr;

    // Steelwall2014: ViewCache is embedded in RHITexture in NilouEngine.
	/** The assigned view cache for this texture (sourced from transient / pooled texture). Never reset. */
	// class FRHITextureViewCache* ViewCache = nullptr;

    std::vector<RDGSubresourceState> SubresourceStates;

};
using RDGTextureRef = TRefCountPtr<RDGTexture>;

using RDGTextureViewDesc = RHITextureViewDesc;
class RDGTextureView : public RDGResource
{
public:
    RDGTextureView(std::string InName, RDGTexture* InTexture, const RDGTextureViewDesc& InDesc) 
        : RDGResource(InName, ERDGResourceType::TextureView)
        , Texture(InTexture)
        , Desc(InDesc)
    { 
        SubresourceRange.MipIndex = InDesc.BaseMipLevel;
        SubresourceRange.NumMips = InDesc.LevelCount;
        SubresourceRange.ArraySlice = InDesc.BaseArrayLayer;
        SubresourceRange.NumArraySlices = InDesc.LayerCount;
        SubresourceRange.PlaneSlice = 0;
        SubresourceRange.NumPlaneSlices = 1;
        if (IsStencilFormat(InTexture->Desc.Format))
        {
            if (IsStencilFormat(InDesc.Format))
            {
                SubresourceRange.PlaneSlice = 0;
                SubresourceRange.NumPlaneSlices = 2;
            }
            else
            {
                Ncheckf(false, "Not supported");
            }
        }
    }

    RDGTexture* Texture;
    const RDGTextureViewDesc Desc;

    RDGTexture* GetParent() const { return Texture; }

    RHITextureView* GetRHI() const { return static_cast<RHITextureView*>(ResourceRHI.GetReference()); }

    uint32 GetSizeX() const { return GetParent()->Desc.SizeX >> Desc.BaseMipLevel; }
    uint32 GetSizeY() const { return GetParent()->Desc.SizeY >> Desc.BaseMipLevel; }
    uint32 GetSizeZ() const { return GetParent()->Desc.SizeZ >> Desc.BaseMipLevel; }

    const FRDGTextureSubresourceRange& GetSubresourceRange() const { return SubresourceRange; }

    FRDGTextureSubresourceRange SubresourceRange;
};
using RDGTextureViewRef = TRefCountPtr<RDGTextureView>;

/************ Buffer *************/
class RDGBuffer : public RDGResource
{
public:
    RDGBuffer(std::string InName, const RDGBufferDesc& InDesc) 
        : RDGResource(InName, ERDGResourceType::Buffer)
        , Desc(InDesc) 
    { 
        // Buffer = std::make_unique<uint8[]>(InDesc.GetSize());
    }
    RHIBuffer* GetRHI() const { return static_cast<RHIBuffer*>(ResourceRHI.GetReference()); }

    uint32 GetSize() const { return Desc.GetSize(); }

    const RDGBufferDesc Desc;

    // Steelwall2014: not null if the buffer is created from RenderGraph::CreateBuffer
    class FRHITransientBuffer* TransientBuffer;
    // Steelwall2014: not null if the buffer is created from RenderGraph::CreateExternalBuffer
    FRDGPooledBufferRef PooledBuffer;

    RDGSubresourceState State;

    bool bQueuedForUpload = false;
};
using RDGBufferRef = TRefCountPtr<RDGBuffer>;

template <typename T>
class TRDGUniformBuffer : public RDGBuffer
{
public:
    TRDGUniformBuffer(std::string InName, const RDGBufferDesc& InDesc) 
        : RDGBuffer(InName, InDesc)
    { 
    }
};
template <typename T>
using TRDGUniformBufferRef = TRefCountPtr<TRDGUniformBuffer<T>>;

struct RDGRenderTargets
{
    struct FBindingPoint
    {
        FBindingPoint() = default;
        FBindingPoint(RDGTextureView* InTextureView) : TextureView(InTextureView) {}

        RDGTextureView* TextureView = nullptr;
        ERenderTargetLoadAction LoadAction = ERenderTargetLoadAction::Load;
        ERenderTargetStoreAction StoreAction = ERenderTargetStoreAction::Store;
    };
    FBindingPoint DepthStencilAttachment;

    std::array<FBindingPoint, MaxSimultaneousRenderTargets> ColorAttachments;

    RHIFramebufferRef FramebufferRHI = nullptr;

    RHIRenderTargetLayout GetRenderTargetLayout() const;
};

}
