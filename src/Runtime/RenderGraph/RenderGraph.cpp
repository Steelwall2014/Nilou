#include "RenderGraph.h"
#include "DynamicRHI.h"
#include "RHICommandList.h"
#include "RHITransition.h"
#include "RHITransientResourceAllocator.h"
#include "RenderGraphResourcePool.h"
#include "RenderGraphTransition.h"
#include "PlatformMisc.h"
#include "Stats/Stats2.h"
#include "Common/Containers/Array.h"

namespace nilou {

std::map<RHIDescriptorSetLayout*, RDGDescriptorSetPool> RenderGraph::DescriptorSetPools;
FRDGBufferPool GRenderGraphBufferPool;
FRDGTexturePool GRenderGraphTexturePool;

/** Enumerates all texture accesses and provides the access and subresource range info. This results in
 *  multiple invocations of the same resource, but with different access / subresource range.
 */
void RenderGraph::EnumerateTextureAccess(const std::vector<RDGDescriptorSet*>& PassParameters, const std::function<void(RDGTextureView*,RDGTexture*,RHISamplerState*,ERHIAccess)>& AccessFunction)
{
    for (RDGDescriptorSet* DescriptorSet : PassParameters)
    {
        for (auto& [BindingIndex, Writer] : DescriptorSet->WriterInfos)
        {
            if (Writer.DescriptorType == EDescriptorType::CombinedImageSampler || Writer.DescriptorType == EDescriptorType::StorageImage)
            {
				RDGTextureView* TextureView = Writer.ImageInfo.Texture;
                RDGTexture* Texture = TextureView->GetParent();
                AccessFunction(TextureView, Texture, Writer.ImageInfo.SamplerState, Writer.Access);
            }
        }
    }
}

/** Enumerates all buffer accesses and provides the access info. */
void RenderGraph::EnumerateBufferAccess(const std::vector<RDGDescriptorSet*>& PassParameters, const std::function<void(RDGBuffer*,ERHIAccess)>& AccessFunction)
{
    for (RDGDescriptorSet* DescriptorSet : PassParameters)
    {
        for (auto [BindingIndex, Writer] : DescriptorSet->WriterInfos)
        {
            if (Writer.DescriptorType == EDescriptorType::StorageBuffer)
            {
                AccessFunction(Writer.BufferInfo.Buffer, Writer.Access);
            }
        }
    }
}

RenderGraph::RenderGraph()
    : TransientResourceAllocator(GRDGTransientResourceAllocator.Get())
{
	RDGPassDesc ProloguePassDesc{"ProloguePass"};
	ProloguePassDesc.bNeverCull = true;
	ProloguePass = new FRDGPass(GetProloguePassHandle(), ProloguePassDesc, ERHIPipeline::Graphics);
	Passes.push_back(ProloguePass);
}

RDGTextureRef RenderGraph::CreateExternalTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    RDGTextureRef Texture = new RDGTexture(Name, Desc);
    Texture->Name = Name;
    Texture->bIsPersistent = true;
	Texture->ResourceRHI = RHICreateTexture(Desc, Name);
	Texture->PooledDefaultView = CreateExternalTextureView(Texture);
    return Texture;
}

RDGTextureViewRef RenderGraph::CreateExternalTextureView(const std::string& Name, RDGTexture* InTexture, const RDGTextureViewDesc& ViewDesc)
{
    RDGTextureViewRef TextureView = new RDGTextureView(Name, InTexture, ViewDesc);
    return TextureView;
}

RDGTextureViewRef RenderGraph::CreateExternalTextureView(RDGTexture* InTexture)
{
	std::string Name = InTexture->Name + "_DefaultView";
	RDGTextureViewDesc ViewDesc;
	ViewDesc.Format = InTexture->Desc.Format;
	ViewDesc.BaseMipLevel = 0;
	ViewDesc.LevelCount = InTexture->Desc.NumMips;
	ViewDesc.BaseArrayLayer = 0;
	ViewDesc.LayerCount = InTexture->Desc.ArraySize;
	ViewDesc.ViewType = InTexture->Desc.TextureType;
    RDGTextureViewRef TextureView = new RDGTextureView(Name, InTexture, ViewDesc);
    return TextureView;
}

RDGBufferRef RenderGraph::CreateExternalBuffer(const std::string& Name, const RDGBufferDesc& Desc)
{
    RDGBufferRef Buffer = new RDGBuffer(Name, Desc);
    Buffer->Name = Name;
    Buffer->bIsPersistent = true;
    Buffer->ResourceRHI = RHICreateBuffer(Desc.GetStride(), Desc.GetStride(), EBufferUsageFlags::None, nullptr);
    return Buffer;
}

RDGDescriptorSetRef RenderGraph::CreateExternalDescriptorSet(RHIDescriptorSetLayout* Layout)
{
    if (DescriptorSetPools.find(Layout) == DescriptorSetPools.end())
    {
        DescriptorSetPools[Layout] = RDGDescriptorSetPool(Layout);
    }

    RDGDescriptorSetPool& Pool = DescriptorSetPools[Layout];
    RDGDescriptorSetRef DescriptorSet = Pool.Allocate();
    return DescriptorSet;
}

RDGTexture* RenderGraph::CreateTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    RDGTextureRef Texture = new RDGTexture(Name, Desc);
	Texture->TransientDefaultView = CreateTextureView(Texture);
    Textures.push_back(Texture);
    return Texture;
}

RDGTextureView* RenderGraph::CreateTextureView(const std::string& Name, RDGTexture* InTexture, const RDGTextureViewDesc& ViewDesc)
{
    RDGTextureViewRef TextureView = new RDGTextureView(Name, InTexture, ViewDesc);
    TextureViews.push_back(TextureView);
    return TextureView;
}

RDGTextureView* RenderGraph::CreateTextureView(RDGTexture* InTexture)
{
	std::string Name = InTexture->Name + "_DefaultView";
	RDGTextureViewDesc ViewDesc;
	ViewDesc.Format = InTexture->Desc.Format;
	ViewDesc.BaseMipLevel = 0;
	ViewDesc.LevelCount = InTexture->Desc.NumMips;
	ViewDesc.BaseArrayLayer = 0;
	ViewDesc.LayerCount = InTexture->Desc.ArraySize;
	ViewDesc.ViewType = InTexture->Desc.TextureType;
    RDGTextureViewRef TextureView = new RDGTextureView(Name, InTexture, ViewDesc);
    TextureViews.push_back(TextureView);
    return TextureView;
}

RDGBuffer* RenderGraph::CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc)
{
    RDGBufferRef Buffer = new RDGBuffer(Name, Desc);
    Buffers.push_back(Buffer);
    return Buffer;
}

RDGDescriptorSet* RenderGraph::CreateDescriptorSet(RHIDescriptorSetLayout* Layout)
{
    if (DescriptorSetPools.find(Layout) == DescriptorSetPools.end())
    {
        DescriptorSetPools[Layout] = RDGDescriptorSetPool(Layout);
    }

    RDGDescriptorSetPool& Pool = DescriptorSetPools[Layout];
    RDGDescriptorSetRef DescriptorSet = Pool.Allocate();
    DescriptorSets.push_back(DescriptorSet);
    return DescriptorSet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::AddPassDependency(FRDGPass* Producer, FRDGPass* Consumer)
{
	Consumer->Producers.insert(Producer);
	Producer->Consumers.insert(Consumer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RenderGraph::SetupPassInternal(FRDGPass* Pass)
{
	AsyncComputePassCount += Pass->Pipeline == ERHIPipeline::AsyncCompute ? 1 : 0;
	RasterPassCount += Pass->Pipeline == ERHIPipeline::Graphics ? 1 : 0;
}

void RenderGraph::SetupParameterPass(FRDGPass* Pass)
{
	SetupPassInternal(Pass);

	SetupPassResources(Pass);
}

void RenderGraph::SetupCopyPass(FRDGPass* Pass, RDGResource* Source, RDGResource* Destination)
{
	SetupPassInternal(Pass);

	SetupCopyPassResource(Pass, Source, ERHIAccess::TransferRead);
	SetupCopyPassResource(Pass, Destination, ERHIAccess::TransferWrite);
}

void RenderGraph::SetupCopyPassResource(FRDGPass* Pass, RDGResource* Resource, ERHIAccess Access)
{
	const FRDGPassHandle PassHandle = Pass->Handle;
	const ERHIPipeline PassPipeline = Pass->Pipeline;
	Ncheck(PassPipeline == ERHIPipeline::Copy);

	if (Resource->Type == ERDGResourceType::Buffer)
	{
		RDGBuffer* Buffer = static_cast<RDGBuffer*>(Resource);
		FRDGPass::FBufferState& PassState = Pass->FindOrAddBufferState(Buffer);
		PassState.ReferenceCount++;

		PassState.Access = Access;
	}
	else if (Resource->Type == ERDGResourceType::TextureView)
	{
		RDGTextureView* TextureView = static_cast<RDGTextureView*>(Resource);
		RDGTexture* Texture = TextureView->GetParent();
		FRDGPass::FTextureState& PassState = Pass->FindOrAddTextureState(Texture);
		PassState.ReferenceCount++;
		
		FRDGTextureSubresourceRange WholeRange = Texture->GetSubresourceRange();
		for (FRDGTextureSubresource& Subresource : TextureView->GetSubresourceRange())
		{
			int32 SubresourceIndex = WholeRange.GetSubresourceIndex(Subresource);
			PassState.Access[SubresourceIndex] = Access;
		}
	}
}

void RenderGraph::SetupPassResources(FRDGPass* Pass)
{
	const FRDGPassHandle PassHandle = Pass->Handle;
	const ERHIPipeline PassPipeline = Pass->Pipeline;

    // Steelwall2014: This lambda function can be called on the same texture multiple times with different access and subresource range.
    // So PassState.ReferenceCount is needed to keep track of the number of times the texture is accessed in this pass.
	EnumerateTextureAccess(Pass->DescriptorSets, [&](RDGTextureView* TextureView, RDGTexture* Texture, RHISamplerState* SamplerState, ERHIAccess Access)
	{
		FRDGPass::FTextureState& PassState = Pass->FindOrAddTextureState(Texture);
		PassState.ReferenceCount++;
		Texture->ReferenceCount++;

		// Enumerate the subresource range of the texture view, 
		// and find the subresource index in the whole texture.
		FRDGTextureSubresourceRange WholeRange = Texture->GetSubresourceRange();
		for (FRDGTextureSubresource& Subresource : TextureView->GetSubresourceRange())
		{
			int32 SubresourceIndex = WholeRange.GetSubresourceIndex(Subresource);
			PassState.Access[SubresourceIndex] = Access;
		}
	});

	EnumerateBufferAccess(Pass->DescriptorSets, [&](RDGBuffer* Buffer, ERHIAccess Access)
	{
		FRDGPass::FBufferState& PassState = Pass->FindOrAddBufferState(Buffer);
		PassState.ReferenceCount++;
		Buffer->ReferenceCount++;

		PassState.Access = Access;
	});
	
	SetupPassDependencies(Pass);
}

void RenderGraph::SetupPassDependencies(FRDGPass* Pass)
{
	bool bIsCullRootProducer = false;

	bIsCullRootProducer |= Pass->Desc.bNeverCull;

	for (FRDGPass::FTextureState& PassState : Pass->TextureStates)
	{
		RDGTexture* Texture = PassState.Texture;
		std::vector<FRDGProducerState>& LastProducers = Texture->LastProducers;

		for (uint32 Index = 0, Count = LastProducers.size(); Index < Count; ++Index)
		{
			if (PassState.Access[Index] == ERHIAccess::None)
			{
				continue;
			}

			if (LastProducers[Index].Pass)
			{
				AddPassDependency(LastProducers[Index].Pass, Pass);
			}

			bIsCullRootProducer |= Texture->IsCullRoot();
		}
	}

	if (bIsCullRootProducer)
	{
		CullPassStack.push_back(Pass);
	}
}

void RenderGraph::Compile()
{
	SCOPE_CYCLE_COUNTER(STAT_RDG_CompileTime);

	const FRDGPassHandle ProloguePassHandle = GetProloguePassHandle();
	const FRDGPassHandle EpiloguePassHandle = GetEpiloguePassHandle();

	while (CullPassStack.size() > 0)
	{
		FRDGPass* Pass = CullPassStack.back(); CullPassStack.pop_back();

		if (Pass->bCulled)
		{
			Pass->bCulled = 0;

			for (FRDGPass* Producer : Pass->Producers)
			{
				CullPassStack.push_back(Producer);
			}
		}
	}

	constexpr bool bCullPasses = true;

	if (bCullPasses)
	{
		SCOPED_NAMED_EVENT(PassCulling, FColor::Emerald);

		// Manually mark the prologue / epilogue passes as not culled.
		EpiloguePass->bCulled = 0;
		ProloguePass->bCulled = 0;

		for (FRDGPassHandle PassHandle = ProloguePassHandle + 1; PassHandle < EpiloguePassHandle; ++PassHandle)
		{
			FRDGPass* Pass = Passes[PassHandle];

			if (!Pass->bCulled)
			{
				continue;
			}

			// Subtract reference counts from culled passes that were added during pass setup.
		
			for (auto& PassState : Pass->TextureStates)
			{
				PassState.Texture->ReferenceCount -= PassState.ReferenceCount;
			}

			for (auto& PassState : Pass->BufferStates)
			{
				PassState.Buffer->ReferenceCount -= PassState.ReferenceCount;
			}
		}
	}
}

void RenderGraph::Execute()
{
	RDGPassDesc EpiloguePassDesc{"EpiloguePass"};
	EpiloguePassDesc.bNeverCull = true;
	EpiloguePass = new FRDGPass(Passes.size(), EpiloguePassDesc, ERHIPipeline::Graphics);
	Passes.push_back(EpiloguePass);

	const FRDGPassHandle ProloguePassHandle = GetProloguePassHandle();
	const FRDGPassHandle EpiloguePassHandle = GetEpiloguePassHandle();

	const int32 NumBuffers           = Buffers.size();
	const int32 NumTextures          = Textures.size();
	const int32 NumExternalBuffers   = ExternalBuffers.size();
	const int32 NumExternalTextures  = ExternalTextures.size();
	FCollectResourceContext CollectResourceContext;

    Compile();

	CollectPassBarriers();

	for  (auto [TextureRHI, Texture] : ExternalTextures)
	{
		if (Texture->IsCulled())
		{
			CollectDeallocateTexture(CollectResourceContext, ProloguePassHandle, Texture, 0);
		}
	}

	for  (auto [BufferRHI, Buffer] : ExternalBuffers)
	{
		if (Buffer->IsCulled())
		{
			CollectDeallocateBuffer(CollectResourceContext, ProloguePassHandle, Buffer, 0);
		}
	}


	// CollectResources
	for (FRDGPass* Pass : Passes)
	{
		if (!Pass->bCulled)
		{
			CollectAllocations(CollectResourceContext, Pass);
			CollectDeallocations(CollectResourceContext, Pass);
		}
	}
	AllocatePooledBuffers(CollectResourceContext.PooledBuffers);
	AllocatePooledTextures(CollectResourceContext.PooledTextures);
	AllocateTransientResources(CollectResourceContext.TransientResources);

	CreateViews(CollectResourceContext.Views);
	
	for (FRDGPassHandle PassHandle = ProloguePassHandle; PassHandle <= EpiloguePassHandle; ++PassHandle)
	{
		FRDGPass* Pass = Passes[PassHandle];

		if (Pass->bCulled)
		{
			continue;
		}
		
		RHICommandList* RHICmdList = nullptr;
		if (Pass->Pipeline == ERHIPipeline::Graphics)
			RHICmdList = RHICreateGfxCommandList();
		else if (Pass->Pipeline == ERHIPipeline::AsyncCompute)
			RHICmdList = RHICreateComputeCommandList();
		else if (Pass->Pipeline == ERHIPipeline::Copy)
			RHICmdList = RHICreateTransferCommandList();
		ExecuteSerialPass(*RHICmdList, Pass);
		RHISubmitCommandList(RHICmdList, Pass->SemaphoresToWait, Pass->SemaphoresToSignal);
	}

}

void RenderGraph::ExecuteSerialPass(RHICommandList& RHICmdList, FRDGPass* Pass)
{
	RHICmdList.PipelineBarrier(Pass->MemoryBarriers, Pass->ImageBarriers, Pass->BufferBarriers);
	if (Pass->Pipeline == ERHIPipeline::Graphics)
	{
		std::array<RHITextureView*, MaxSimultaneousRenderTargets> ColorAttachments = { nullptr };
		RHITextureView* DepthStencilAttachment = nullptr;
		FRHIRenderPassInfo Info;
		Info.Extent = ivec2(0, 0);
		std::map<int, int&> map;
		auto iter = map.begin();
		for (auto [Index, ColorBindingPoint] : Enumerate(Pass->RenderTargets.ColorAttachments))
		{
			auto ColorRDG = ColorBindingPoint.TextureView;
			if (ColorRDG)
			{
				ColorAttachments[Index] = ColorRDG->GetRHI();
				Info.Extent = ivec2(ColorRDG->GetSizeX(), ColorRDG->GetSizeY());
			}
		}
		auto& DepthStencilRDG = Pass->RenderTargets.DepthStencilAttachment.TextureView;
		if (DepthStencilRDG)
		{
			DepthStencilAttachment = DepthStencilRDG->GetRHI();
			Info.Extent = ivec2(DepthStencilRDG->GetSizeX(), DepthStencilRDG->GetSizeY());
		}
		RHICmdList.BeginRenderPass(Info);
	}
	Pass->Execute(RHICmdList);
}

/******************** Collect Resources ********************/

void RenderGraph::CollectAllocations(FCollectResourceContext& Context, FRDGPass* Pass)
{
    for (FRDGPass::FBufferState& PassState : Pass->BufferStates)
    {
        CollectAllocateBuffer(Context, Pass->Handle, PassState.Buffer);
    }

    for (FRDGPass::FTextureState& PassState : Pass->TextureStates)
    {
        CollectAllocateTexture(Context, Pass->Handle, PassState.Texture);
    }

	for (RDGTextureView* View : Pass->Views)
	{
		Context.Views.insert(View);
	}
}

void RenderGraph::CollectAllocateTexture(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGTexture* Texture)
{
	if (Texture->FirstPass == NullPassHandle)
	{
		Texture->FirstPass = PassHandle;
	}

    if (Texture->bCollectForAllocate)
    {
        Texture->bCollectForAllocate = false;
		Ncheck(!Texture->ResourceRHI);

		const FCollectResourceOp AllocateOp = FCollectResourceOp::Allocate(PassHandle, Texture);

		if (Texture->bTransient)
		{
			Context.TransientResources.emplace_back(AllocateOp);
		}
		else
		{
			Context.PooledTextures.emplace_back(AllocateOp);
		}
    }
}

void RenderGraph::CollectAllocateBuffer(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGBuffer* Buffer)
{
	if (Buffer->FirstPass == NullPassHandle)
	{
		Buffer->FirstPass = PassHandle;
	}

    if (Buffer->bCollectForAllocate)
    {
        Buffer->bCollectForAllocate = false;
		Ncheck(!Buffer->ResourceRHI);

		const FCollectResourceOp AllocateOp = FCollectResourceOp::Allocate(PassHandle, Buffer);

		if (Buffer->bTransient)
		{
			Context.TransientResources.emplace_back(AllocateOp);
		}
		else
		{
			Context.PooledBuffers.emplace_back(AllocateOp);
		}
    }
    
}

void RenderGraph::CollectDeallocations(FCollectResourceContext& Context, FRDGPass* Pass)
{
    for (FRDGPass::FBufferState& PassState : Pass->BufferStates)
    {
        CollectDeallocateBuffer(Context, Pass->Handle, PassState.Buffer, PassState.ReferenceCount);
    }

    for (FRDGPass::FTextureState& PassState : Pass->TextureStates)
    {
        CollectDeallocateTexture(Context, Pass->Handle, PassState.Texture, PassState.ReferenceCount);
    }
}

void RenderGraph::CollectDeallocateTexture(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGTexture* Texture, uint32 ReferenceCount)
{
	Ncheck(Texture->ReferenceCount != RDGResource::DeallocatedReferenceCount);
	Ncheck(Texture->ReferenceCount >= ReferenceCount);
	Texture->ReferenceCount -= ReferenceCount;

	if (Texture->ReferenceCount == 0)
	{
		const FCollectResourceOp DeallocateOp = FCollectResourceOp::Deallocate(PassHandle, Texture);

		if (Texture->bTransient)
		{
			Context.TransientResources.emplace_back(DeallocateOp);
		}
		else
		{
			Context.PooledTextures.emplace_back(DeallocateOp);
		}

		Texture->LastPass = PassHandle;
		Texture->ReferenceCount = RDGResource::DeallocatedReferenceCount;
	}
}

void RenderGraph::CollectDeallocateBuffer(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGBuffer* Buffer, uint32 ReferenceCount)
{
	Ncheck(Buffer->ReferenceCount != RDGResource::DeallocatedReferenceCount);
	Ncheck(Buffer->ReferenceCount >= ReferenceCount);
	Buffer->ReferenceCount -= ReferenceCount;

	if (Buffer->ReferenceCount == 0)
	{
		const FCollectResourceOp DeallocateOp = FCollectResourceOp::Deallocate(PassHandle, Buffer);

		if (Buffer->bTransient)
		{
			Context.TransientResources.emplace_back(DeallocateOp);
		}
		else
		{
			Context.PooledBuffers.emplace_back(DeallocateOp);
		}

		Buffer->LastPass = PassHandle;
		Buffer->ReferenceCount = RDGResource::DeallocatedReferenceCount;
	}
}


/************** Pooled Resources Allocation ****************/

FRDGPooledTextureRef RenderGraph::AllocatePooledRenderTargetRHI(RDGTexture* Texture)
{
	return GRenderGraphTexturePool.FindFreeElement(Texture->Desc, Texture->Name);
}

FRDGPooledBufferRef RenderGraph::AllocatePooledBufferRHI(RDGBuffer* Buffer)
{
	return GRenderGraphBufferPool.FindFreeBuffer(Buffer->Desc, Buffer->Name);
}

void RenderGraph::AllocatePooledTextures(const std::vector<FCollectResourceOp>& Ops)
{
	SCOPED_NAMED_EVENT("FRDGBuilder::AllocatePooledTextures", FColor::Magenta);
	std::lock_guard<std::recursive_mutex> Lock(GRenderGraphTexturePool.Mutex);

	for (FCollectResourceOp Op : Ops)
	{
		RDGTexture* Texture = static_cast<RDGTexture*>(Op.Resource);

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			FRDGPooledTextureRef PooledTexture = AllocatePooledRenderTargetRHI(Texture);
			Texture->ResourceRHI = PooledTexture->GetRHI();
			Texture->PooledTexture = PooledTexture;
			break;
		}
		case FCollectResourceOp::EOp::Deallocate:
			Texture->PooledTexture = nullptr;
			Texture->ResourceRHI = nullptr;
			break;
		}
	}

}

void RenderGraph::AllocatePooledBuffers(const std::vector<FCollectResourceOp>& Ops)
{
	SCOPED_NAMED_EVENT("FRDGBuilder::AllocatePooledBuffers", FColor::Magenta);
	std::lock_guard<std::recursive_mutex> Lock(GRenderGraphBufferPool.Mutex);

	for (FCollectResourceOp Op : Ops)
	{
		RDGBuffer* Buffer = static_cast<RDGBuffer*>(Op.Resource);

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			FRDGPooledBufferRef PooledBuffer = AllocatePooledBufferRHI(Buffer);
			Buffer->ResourceRHI = PooledBuffer->GetRHI();
			Buffer->PooledBuffer = PooledBuffer;
			break;
		}
		case FCollectResourceOp::EOp::Deallocate:
			Buffer->ResourceRHI = nullptr;
			Buffer->PooledBuffer = nullptr;
			break;
		}
	}

}

void RenderGraph::CreateViews(const std::unordered_set<RDGTextureView*>& ViewsToCreate)
{
	for (RDGTextureView* View : ViewsToCreate)
	{
		RHITexture* TextureRHI = View->GetParent()->GetRHI();
		if (TextureRHI)
		{
			RHITextureView* ViewRHI = TextureRHI->GetOrCreateView(View->Desc);
			View->ResourceRHI = ViewRHI;
		}
	}
}

/******************** Transient Resources Allocation ******************/

void RenderGraph::SetTransientTextureRHI(RDGTexture* Texture, FRHITransientTexture* TransientTexture)
{
	Texture->ResourceRHI = TransientTexture->GetRHI();
	Texture->TransientTexture = TransientTexture;
}

void RenderGraph::SetTransientBufferRHI(RDGBuffer* Buffer, FRHITransientBuffer* TransientBuffer)
{
	Buffer->ResourceRHI = TransientBuffer->GetRHI();
	Buffer->TransientBuffer = TransientBuffer;
}

void RenderGraph::AllocateTransientResources(const std::vector<FCollectResourceOp>& Ops)
{
	for (FCollectResourceOp Op : Ops)
	{
		const FRDGPassHandle PassHandle = Op.PassHandle;

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			if (Op.Resource->Type == ERDGResourceType::Buffer)
			{
				RDGBuffer* Buffer = static_cast<RDGBuffer*>(Op.Resource);
				FRHITransientBuffer* TransientBuffer = TransientResourceAllocator->CreateBuffer(Buffer->Desc.Translate(), Buffer->Name, PassHandle);

				SetTransientBufferRHI(Buffer, TransientBuffer);

				Buffer->MinAcquirePass = FRDGPassHandle(TransientBuffer->GetAcquirePasses().Min);
			}
			else
			{
				RDGTexture* Texture = static_cast<RDGTexture*>(Op.Resource);
				FRHITransientTexture* TransientTexture = TransientResourceAllocator->CreateTexture(Texture->Desc, Texture->Name, PassHandle);

                SetTransientTextureRHI(Texture, TransientTexture);

				Texture->MinAcquirePass = FRDGPassHandle(TransientTexture->GetAcquirePasses().Min);
			}
		}
		break;
		case FCollectResourceOp::EOp::Deallocate:
		{
			if (Op.Resource->Type == ERDGResourceType::Buffer)
			{
				RDGBuffer* Buffer = static_cast<RDGBuffer*>(Op.Resource);
				FRHITransientBuffer* TransientBuffer = Buffer->TransientBuffer;
				TransientResourceAllocator->DeallocateMemory(TransientBuffer, PassHandle);

				Buffer->MinDiscardPass = FRDGPassHandle(TransientBuffer->GetDiscardPasses().Min);
			}
			else
			{
				RDGTexture* Texture = static_cast<RDGTexture*>(Op.Resource);
				FRHITransientTexture* TransientTexture = Texture->TransientTexture;

				// Texture is using an internal transient texture.
				TransientResourceAllocator->DeallocateMemory(TransientTexture, PassHandle);

				if (!TransientTexture->IsAcquired())
				{
					Texture->MinDiscardPass = FRDGPassHandle(TransientTexture->GetDiscardPasses().Min);
				}
			}
		}
		break;
		}
    }
}

/******************** Resource Barriers ******************/

void RenderGraph::CollectPassBarriers()
{
	SCOPED_NAMED_EVENT("FRDGBuilder::CollectBarriers", FColor::Magenta);
	SCOPE_CYCLE_COUNTER(STAT_RDG_CollectBarriersTime);

	for (FRDGPassHandle PassHandle = GetProloguePassHandle() + 1; PassHandle < GetEpiloguePassHandle(); ++PassHandle)
	{
		CollectPassBarriers(PassHandle);
	}
}

EPipelineStageFlags GetPipelineStage(ERHIAccess Access)
{
	switch (Access)
	{
	case ERHIAccess::None:
		return EPipelineStageFlags::None;
	case ERHIAccess::IndirectCommandRead:
		return EPipelineStageFlags::DrawIndirect;
	case ERHIAccess::IndexRead:
	case ERHIAccess::VertexAttributeRead:
		return EPipelineStageFlags::VertexInput;
	case ERHIAccess::UniformRead:
	case ERHIAccess::ShaderResourceRead:
	case ERHIAccess::ShaderResourceWrite:
	case ERHIAccess::ShaderResourceReadWrite:
		return EPipelineStageFlags::FragmentShader;
	case ERHIAccess::ColorAttachmentRead:
	case ERHIAccess::ColorAttachmentWrite:
		return EPipelineStageFlags::ColorAttachmentOutput;
	case ERHIAccess::DepthStencilAttachmentRead:
	case ERHIAccess::DepthStencilAttachmentWrite:
		return EPipelineStageFlags::EarlyFragmentTests;
	case ERHIAccess::TransferRead:
	case ERHIAccess::TransferWrite:
		return EPipelineStageFlags::Transfer;
	case ERHIAccess::HostRead:
	case ERHIAccess::HostWrite:
		return EPipelineStageFlags::Host;
	default:
		Ncheckf(false, "Unknown access type {}", (int)Access);
	};
	return EPipelineStageFlags::None;
}
ETextureLayout GetTextureLayout(ERHIAccess Access)
{
	switch (Access)
	{
	case ERHIAccess::None:
		return ETextureLayout::Undefined;
	case ERHIAccess::ShaderResourceRead:
		return ETextureLayout::ShaderReadOnlyOptimal;
	case ERHIAccess::ColorAttachmentRead:
	case ERHIAccess::ColorAttachmentWrite:
		return ETextureLayout::ColorAttachmentOptimal;
	case ERHIAccess::DepthStencilAttachmentRead:
		return ETextureLayout::DepthStencilReadOnlyOptimal;
	case ERHIAccess::DepthStencilAttachmentWrite:
		return ETextureLayout::DepthStencilAttachmentOptimal;
	case ERHIAccess::TransferRead:
		return ETextureLayout::TransferSrcOptimal;
	case ERHIAccess::TransferWrite:
		return ETextureLayout::TransferDstOptimal;
	case ERHIAccess::HostRead:
	case ERHIAccess::HostWrite:
		return ETextureLayout::General;
	default:
		Ncheckf(false, "Invalid access type {}", (int)Access);
	};
	return ETextureLayout::Undefined;
}

void RenderGraph::CollectPassBarriers(FRDGPassHandle PassHandle)
{
	FRDGPass* CurrentPass = Passes[PassHandle];

	if (CurrentPass->bCulled)
	{
		return;
	}

	const ERHIPipeline CurrentPipeline = CurrentPass->Pipeline;

	for (auto& PassState : CurrentPass->TextureStates)
	{
		RDGTexture* Texture = PassState.Texture;

		int NumSubresources = Texture->GetSubresourceCount();
		for (int32 Index = 0; Index < NumSubresources; ++Index)
		{
			if (PassState.Access[Index] == ERHIAccess::None)
			{
				continue;
			}
			RDGSubresourceState& LastState = Texture->SubresourceStates[Index];
			FRDGPass* LastPass = Passes[LastState.Pass];
			ERHIAccess LastAccess = LastState.Access;
			ERHIAccess CurrentAccess = PassState.Access[Index];
			
			if (LastAccess != CurrentAccess)
			{
				const ERHIPipeline LastPipeline = LastPass->Pipeline;
				RHIImageMemoryBarrier Barrier = RHIImageMemoryBarrier(
					Texture->GetRHI(), 
					LastAccess, CurrentAccess, 
					GetPipelineStage(LastAccess), GetPipelineStage(CurrentAccess),
					GetTextureLayout(LastAccess), GetTextureLayout(CurrentAccess),
					Texture->GetSubresource(Index));
				CurrentPass->ImageBarriers.push_back(Barrier);
				if (LastPipeline != CurrentPipeline)
				{
					RHISemaphoreRef Semaphore = RHICreateSemaphore();
					LastPass->SemaphoresToSignal.push_back(Semaphore);
					CurrentPass->SemaphoresToWait.push_back(Semaphore);
				}
			}

			LastState.Access = CurrentAccess;
			LastState.Pass = CurrentPass->Handle;
		}
	}

	for (auto& PassState : CurrentPass->BufferStates)
	{
		RDGBuffer* Buffer = PassState.Buffer;

		if (PassState.Access == ERHIAccess::None)
		{
			continue;
		}
		RDGSubresourceState& LastState = Buffer->State;
		FRDGPass* LastPass = Passes[LastState.Pass];
		ERHIAccess LastAccess = LastState.Access;
		ERHIAccess CurrentAccess = PassState.Access;

		if (LastAccess != CurrentAccess)
		{
			const ERHIPipeline LastPipeline = LastPass->Pipeline;
			RHIBufferMemoryBarrier Barrier = RHIBufferMemoryBarrier(
				Buffer->GetRHI(), 
				LastAccess, CurrentAccess, 
				GetPipelineStage(LastAccess), GetPipelineStage(CurrentAccess),
				0, Buffer->Desc.GetSize());
			CurrentPass->BufferBarriers.push_back(Barrier);
			if (LastPipeline != CurrentPipeline)
			{
				RHISemaphoreRef Semaphore = RHICreateSemaphore();
				LastPass->SemaphoresToSignal.push_back(Semaphore);
				CurrentPass->SemaphoresToWait.push_back(Semaphore);
			}
		}

		LastState.Access = CurrentAccess;
		LastState.Pass = CurrentPass->Handle;
	}
}

}