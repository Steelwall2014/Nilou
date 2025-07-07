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

bool GRenderGraphDebug = true;
#define RDG_DEBUG_LOG(Verbosity, Format, ...) NILOU_CLOG(GRenderGraphDebug, Verbosity, Format, __VA_ARGS__)

std::map<RHIDescriptorSetLayout*, RHIDescriptorSetPools> RenderGraph::DescriptorSetPools;
FRDGBufferPool GRenderGraphBufferPool;
FRDGTexturePool GRenderGraphTexturePool;

static std::string join(const std::vector<std::string>& Strings, const std::string& Separator = ", ")
{
	std::string Result;
	for (int i = 0; i < Strings.size(); i++)
	{
		Result += Strings[i];
		if (i < Strings.size() - 1)
		{
			Result += Separator;
		}
	}
	return Result;
}

static std::string GetAccessName(ERHIAccess Access)
{
	if (Access == ERHIAccess::None)
	{
		return "None";
	}
	std::vector<std::string> AccessNames;
	if ((Access & ERHIAccess::IndirectCommandRead) != ERHIAccess::None) AccessNames.push_back("IndirectCommandRead");
	if ((Access & ERHIAccess::IndexRead) != ERHIAccess::None) AccessNames.push_back("IndexRead");
	if ((Access & ERHIAccess::VertexAttributeRead) != ERHIAccess::None) AccessNames.push_back("VertexAttributeRead");
	if ((Access & ERHIAccess::UniformRead) != ERHIAccess::None) AccessNames.push_back("UniformRead");
	if ((Access & ERHIAccess::ShaderResourceRead) != ERHIAccess::None) AccessNames.push_back("ShaderResourceRead");
	if ((Access & ERHIAccess::ShaderResourceWrite) != ERHIAccess::None) AccessNames.push_back("ShaderResourceWrite");
	if ((Access & ERHIAccess::ColorAttachmentRead) != ERHIAccess::None) AccessNames.push_back("ColorAttachmentRead");
	if ((Access & ERHIAccess::ColorAttachmentWrite) != ERHIAccess::None) AccessNames.push_back("ColorAttachmentWrite");
	if ((Access & ERHIAccess::DepthStencilAttachmentRead) != ERHIAccess::None) AccessNames.push_back("DepthStencilAttachmentRead");
	if ((Access & ERHIAccess::DepthStencilAttachmentWrite) != ERHIAccess::None) AccessNames.push_back("DepthStencilAttachmentWrite");
	if ((Access & ERHIAccess::TransferRead) != ERHIAccess::None) AccessNames.push_back("TransferRead");
	if ((Access & ERHIAccess::TransferWrite) != ERHIAccess::None) AccessNames.push_back("TransferWrite");
	if ((Access & ERHIAccess::HostRead) != ERHIAccess::None) AccessNames.push_back("HostRead");
	if ((Access & ERHIAccess::HostWrite) != ERHIAccess::None) AccessNames.push_back("HostWrite");
	if ((Access & ERHIAccess::Present) != ERHIAccess::None) AccessNames.push_back("Present");
	return join(AccessNames, " | ");
}

static std::string GetPipelineStageName(EPipelineStageFlags PipelineStage)
{
	if (PipelineStage == EPipelineStageFlags::None)
	{
		return "None";
	}
	std::vector<std::string> PipelineStageNames;
	if ((PipelineStage & EPipelineStageFlags::TopOfPipe) != EPipelineStageFlags::None) PipelineStageNames.push_back("TopOfPipe");
	if ((PipelineStage & EPipelineStageFlags::DrawIndirect) != EPipelineStageFlags::None) PipelineStageNames.push_back("DrawIndirect");
	if ((PipelineStage & EPipelineStageFlags::VertexInput) != EPipelineStageFlags::None) PipelineStageNames.push_back("VertexInput");
	if ((PipelineStage & EPipelineStageFlags::VertexShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("VertexShader");
	if ((PipelineStage & EPipelineStageFlags::TessellationControlShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("TessellationControlShader");
	if ((PipelineStage & EPipelineStageFlags::TessellationEvaluationShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("TessellationEvaluationShader");
	if ((PipelineStage & EPipelineStageFlags::GeometryShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("GeometryShader");
	if ((PipelineStage & EPipelineStageFlags::FragmentShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("FragmentShader");
	if ((PipelineStage & EPipelineStageFlags::EarlyFragmentTests) != EPipelineStageFlags::None) PipelineStageNames.push_back("EarlyFragmentTests");
	if ((PipelineStage & EPipelineStageFlags::LateFragmentTests) != EPipelineStageFlags::None) PipelineStageNames.push_back("LateFragmentTests");
	if ((PipelineStage & EPipelineStageFlags::ColorAttachmentOutput) != EPipelineStageFlags::None) PipelineStageNames.push_back("ColorAttachmentOutput");
	if ((PipelineStage & EPipelineStageFlags::ComputeShader) != EPipelineStageFlags::None) PipelineStageNames.push_back("ComputeShader");
	if ((PipelineStage & EPipelineStageFlags::Transfer) != EPipelineStageFlags::None) PipelineStageNames.push_back("Transfer");
	if ((PipelineStage & EPipelineStageFlags::BottomOfPipe) != EPipelineStageFlags::None) PipelineStageNames.push_back("BottomOfPipe");
	if ((PipelineStage & EPipelineStageFlags::Host) != EPipelineStageFlags::None) PipelineStageNames.push_back("Host");
	if ((PipelineStage & EPipelineStageFlags::AllGraphics) != EPipelineStageFlags::None) PipelineStageNames.push_back("AllGraphics");
	if ((PipelineStage & EPipelineStageFlags::AllCommands) != EPipelineStageFlags::None) PipelineStageNames.push_back("AllCommands");
	return join(PipelineStageNames, " | ");
}

static std::string GetTextureLayoutName(ETextureLayout Layout)
{
	switch (Layout)
	{
	case ETextureLayout::Undefined:
		return "Undefined";
	case ETextureLayout::General:
		return "General";
	case ETextureLayout::ColorAttachmentOptimal:
		return "ColorAttachmentOptimal";
	case ETextureLayout::DepthStencilAttachmentOptimal:
		return "DepthStencilAttachmentOptimal";
	case ETextureLayout::DepthStencilReadOnlyOptimal:
		return "DepthStencilReadOnlyOptimal";
	case ETextureLayout::ShaderReadOnlyOptimal:
		return "ShaderReadOnlyOptimal";
	case ETextureLayout::TransferSrcOptimal:
		return "TransferSrcOptimal";
	case ETextureLayout::TransferDstOptimal:
		return "TransferDstOptimal";
	case ETextureLayout::Preinitialized:
		return "Preinitialized";
	case ETextureLayout::PresentSrc:
		return "PresentSrc";
	default:
		return "Undefined";
	}
}

/** Enumerates all texture accesses and provides the access and subresource range info. This results in
 *  multiple invocations of the same resource, but with different access / subresource range.
 */
void RenderGraph::EnumerateTextureAccess(FRDGPass* Pass, const std::function<void(RDGTextureView*,RDGTexture*,RHISamplerState*,ERHIAccess)>& AccessFunction)
{
    for (RDGDescriptorSet* DescriptorSet : Pass->DescriptorSets)
    {
        for (auto& [BindingIndex, Writer] : DescriptorSet->WriterInfos)
        {
            if (Writer.DescriptorType == EDescriptorType::CombinedImageSampler || Writer.DescriptorType == EDescriptorType::StorageImage)
            {
				RDGTextureView* TextureView = Writer.ImageInfo.Texture;
				RDGTexture* Texture = TextureView->GetParent();
				RDGTextureDesc& Desc = const_cast<RDGTextureDesc&>(Texture->Desc);
				if (Writer.DescriptorType == EDescriptorType::CombinedImageSampler)
				{
					Desc.Usage |= ETextureUsageFlags::Sampled;
				}
				else if (Writer.DescriptorType == EDescriptorType::StorageImage)
				{
					Desc.Usage |= ETextureUsageFlags::Storage;
				}
                AccessFunction(TextureView, Texture, Writer.ImageInfo.SamplerState, Writer.Access);
            }
        }
    }
	for (auto& Attachment : Pass->RenderTargets.ColorAttachments)
	{
		RDGTextureView* TextureView = Attachment.TextureView;
		if (TextureView)
		{
			RDGTexture* Texture = TextureView->GetParent();
			RDGTextureDesc& Desc = const_cast<RDGTextureDesc&>(Texture->Desc);
			Desc.Usage |= ETextureUsageFlags::ColorAttachment;
			AccessFunction(TextureView, Texture, nullptr, ERHIAccess::ColorAttachmentWrite);
		}
	}
	auto& Attachment = Pass->RenderTargets.DepthStencilAttachment;
	RDGTextureView* TextureView = Attachment.TextureView;
	if (TextureView)
	{
		RDGTexture* Texture = TextureView->GetParent();
		RDGTextureDesc& Desc = const_cast<RDGTextureDesc&>(Texture->Desc);
		Desc.Usage |= ETextureUsageFlags::DepthStencilAttachment;
		AccessFunction(TextureView, Texture, nullptr, ERHIAccess::DepthStencilAttachmentWrite);
	}
}

/** Enumerates all buffer accesses and provides the access info. */
void RenderGraph::EnumerateBufferAccess(FRDGPass* Pass, const std::function<void(RDGBuffer*,ERHIAccess)>& AccessFunction)
{
    for (RDGDescriptorSet* DescriptorSet : Pass->DescriptorSets)
    {
        for (auto [BindingIndex, Writer] : DescriptorSet->WriterInfos)
        {
            if (Writer.DescriptorType == EDescriptorType::StorageBuffer)
            {
				RDGBufferDesc& Desc = const_cast<RDGBufferDesc&>(Writer.BufferInfo.Buffer->Desc);
                Desc.Usage |= EBufferUsageFlags::StorageBuffer;
                AccessFunction(Writer.BufferInfo.Buffer, Writer.Access);
            }
            else if (Writer.DescriptorType == EDescriptorType::UniformBuffer)
            {
				RDGBufferDesc& Desc = const_cast<RDGBufferDesc&>(Writer.BufferInfo.Buffer->Desc);
                Desc.Usage |= EBufferUsageFlags::UniformBuffer;
                AccessFunction(Writer.BufferInfo.Buffer, Writer.Access);
			}
        }
    }
	for (RDGBuffer* Buffer : Pass->IndexBuffers)
	{
		RDGBufferDesc& Desc = const_cast<RDGBufferDesc&>(Buffer->Desc);
		Desc.Usage |= EBufferUsageFlags::IndexBuffer;
		AccessFunction(Buffer, ERHIAccess::IndexRead);
	}
	for (RDGBuffer* Buffer : Pass->VertexBuffers)
	{
		RDGBufferDesc& Desc = const_cast<RDGBufferDesc&>(Buffer->Desc);
		Desc.Usage |= EBufferUsageFlags::VertexBuffer;
		AccessFunction(Buffer, ERHIAccess::VertexAttributeRead);
	}
}

EPipelineStageFlags GetPipelineStage(ERHIPipeline Pipeline, ERHIAccess Access)
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
	{
		if (Pipeline == ERHIPipeline::Graphics)
		{
			return EPipelineStageFlags::FragmentShader;
		}
		else
		{
			return EPipelineStageFlags::ComputeShader;
		}
	}
	case ERHIAccess::ShaderResourceRead:
	case ERHIAccess::ShaderResourceWrite:
	case ERHIAccess::ShaderResourceReadWrite:
		return EPipelineStageFlags::ComputeShader;
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
	case ERHIAccess::Present:
		return EPipelineStageFlags::BottomOfPipe;
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
	case ERHIAccess::ShaderResourceWrite:
	case ERHIAccess::ShaderResourceReadWrite:
		return ETextureLayout::General;
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
	case ERHIAccess::Present:
		return ETextureLayout::PresentSrc;
	default:
		Ncheckf(false, "Invalid access type {}", (int)Access);
	};
	return ETextureLayout::Undefined;
}

RenderGraph::RenderGraph()
    : TransientResourceAllocator(GRDGTransientResourceAllocator.Get())
{
	RDGPassDesc ProloguePassDesc{"ProloguePass"};
	ProloguePassDesc.bNeverCull = true;
	ProloguePass = new FRDGPass(GetProloguePassHandle(), ProloguePassDesc, ERHIPipeline::Graphics);
	Passes.push_back(ProloguePass);
}

RenderGraph::~RenderGraph()
{
	for (auto& Pass : Passes)
	{
		delete Pass;
	}
}

void RenderGraph::BeginFrame()
{
	RHIBeginFrame();
	RHITexture* SwapChainTextureRHI = RHIGetSwapChainTexture();
	SwapChainTexture = RegisterExternalTexture("SwapChainTexture", SwapChainTextureRHI);
}

void RenderGraph::EndFrame()
{
	for (RDGTexture* Texture : PooledTextures)
	{
		Texture->ReferenceCount = 0;
		Texture->PassStateIndex = 0;
		Texture->LastPass = NullPassHandle;
		Texture->FirstPass = NullPassHandle;
		Texture->LastProducers.clear();
		Texture->SubresourceStates.clear();
	}
	for (RDGBuffer* Buffer : PooledBuffers)
	{
		Buffer->ReferenceCount = 0;
		Buffer->PassStateIndex = 0;
		Buffer->LastPass = NullPassHandle;
		Buffer->FirstPass = NullPassHandle;
		Buffer->State = RDGSubresourceState();
	}
	RHIEndFrame();
}

RDGTextureRef RenderGraph::CreateExternalTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    RDGTextureRef Texture = new RDGTexture(Name, Desc);
    Texture->Name = Name;
    Texture->bTransient = false;
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
    Buffer->bTransient = false;
    return Buffer;
}

RDGDescriptorSetRef RenderGraph::CreateExternalDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout)
{
    RDGDescriptorSetRef DescriptorSet = new RDGDescriptorSet(Name, Layout);
    return DescriptorSet;
}

RDGTexture* RenderGraph::RegisterExternalTexture(const std::string& Name, RHITexture* TextureRHI)
{
	if (ExternalTextures.find(TextureRHI) != ExternalTextures.end())
	{
		return ExternalTextures[TextureRHI];
	}
	RDGTextureDesc Desc = TextureRHI->GetDesc();
    RDGTexture* Texture = new RDGTexture(Name, Desc);
    Texture->bTransient = true;
    Texture->bExternal = true;
    Texture->bCollectForAllocate = false;
	Texture->ResourceRHI = TextureRHI;
	Texture->TransientDefaultView = CreateTextureView(Texture);
    Textures.push_back(Texture);
	ExternalTextures[TextureRHI] = Texture;
    return Texture;
}

RDGTexture* RenderGraph::CreateTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    RDGTextureRef Texture = new RDGTexture(Name, Desc);
	Texture->bTransient = true;
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
	Buffer->bTransient = true;
    Buffers.push_back(Buffer);
    return Buffer;
}

void RenderGraph::QueueBufferUpload(RDGBuffer* Buffer, const void* InitialData, uint32 InitialDataSize)
{
	if (!InitialData) return;
	Ncheck(InitialDataSize <= Buffer->GetSize());

	std::unique_ptr<uint8[]> InitialDataCopy = std::make_unique<uint8[]>(InitialDataSize);
	std::memcpy(InitialDataCopy.get(), InitialData, InitialDataSize);
	
	RDGPassDesc PassDesc("QueueBufferUpload");
	PassDesc.bNeverCull = true;
	AddCopyPass(
		PassDesc,
		nullptr,
		Buffer,
		[=, InitialDataCopy = std::move(InitialDataCopy)](RHICommandList& RHICmdList)
		{
			RHIBuffer* StagingBuffer = RHICmdList.AcquireStagingBuffer(InitialDataSize);
			void* Data = RHIMapMemory(StagingBuffer, 0, InitialDataSize);
				memcpy(Data, InitialDataCopy.get(), InitialDataSize);
			RHIUnmapMemory(StagingBuffer);
			RHICmdList.CopyBuffer(StagingBuffer, Buffer->GetRHI(), 0, 0, InitialDataSize);
		});
}

RDGDescriptorSet* RenderGraph::CreateDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout)
{
    RDGDescriptorSetRef DescriptorSet = new RDGDescriptorSet(Name, Layout);
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

	if (Source)
	{
		SetupCopyPassResource(Pass, Source, ERHIAccess::TransferRead);
	}
	if (Destination)
	{
		SetupCopyPassResource(Pass, Destination, ERHIAccess::TransferWrite);
	}
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
		Buffer->ReferenceCount++;

		RDGBufferDesc& Desc = const_cast<RDGBufferDesc&>(Buffer->Desc);
		Desc.Usage |= Access == ERHIAccess::TransferRead ? EBufferUsageFlags::TransferSrc : EBufferUsageFlags::TransferDst;

		PassState.Access = Access;
	}
	else if (Resource->Type == ERDGResourceType::TextureView)
	{
		RDGTextureView* TextureView = static_cast<RDGTextureView*>(Resource);
		RDGTexture* Texture = TextureView->GetParent();
		FRDGPass::FTextureState& PassState = Pass->FindOrAddTextureState(Texture);
		PassState.ReferenceCount++;
		Texture->ReferenceCount++;

		RDGTextureDesc& Desc = const_cast<RDGTextureDesc&>(Texture->Desc);
		Desc.Usage |= Access == ERHIAccess::TransferRead ? ETextureUsageFlags::TransferSrc : ETextureUsageFlags::TransferDst;
		
		FRDGTextureSubresourceRange WholeRange = Texture->GetSubresourceRange();
		for (FRDGTextureSubresource& Subresource : TextureView->GetSubresourceRange())
		{
			int32 SubresourceIndex = WholeRange.GetSubresourceIndex(Subresource);
			PassState.Access[SubresourceIndex] = Access;
		}
	}
	else if (Resource->Type == ERDGResourceType::Texture)
	{
		RDGTexture* Texture = static_cast<RDGTexture*>(Resource);
		FRDGPass::FTextureState& PassState = Pass->FindOrAddTextureState(Texture);
		PassState.ReferenceCount++;
		Texture->ReferenceCount++;

		RDGTextureDesc& Desc = const_cast<RDGTextureDesc&>(Texture->Desc);
		Desc.Usage |= Access == ERHIAccess::TransferRead ? ETextureUsageFlags::TransferSrc : ETextureUsageFlags::TransferDst;

		for (int32 SubresourceIndex = 0; SubresourceIndex < Texture->GetSubresourceCount(); ++SubresourceIndex)
		{
			PassState.Access[SubresourceIndex] = Access;
		}
	}
	else 
	{
		Ncheckf(false, "Unsupported resource type: {}", magic_enum::enum_name(Resource->Type));
	}
}

void RenderGraph::SetupPassResources(FRDGPass* Pass)
{
	const FRDGPassHandle PassHandle = Pass->Handle;
	const ERHIPipeline PassPipeline = Pass->Pipeline;

    // Steelwall2014: This lambda function can be called on the same texture multiple times with different access and subresource range.
    // So PassState.ReferenceCount is needed to keep track of the number of times the texture is accessed in this pass.
	EnumerateTextureAccess(Pass, [&](RDGTextureView* TextureView, RDGTexture* Texture, RHISamplerState* SamplerState, ERHIAccess Access)
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
		Pass->Views.push_back(TextureView);
	});

	EnumerateBufferAccess(Pass, [&](RDGBuffer* Buffer, ERHIAccess Access)
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

void RenderGraph::SubmitBufferUploads()
{
	SCOPED_NAMED_EVENT("RenderGraph::SubmitBufferUploads", FColor::Magenta);

	{
		SCOPED_NAMED_EVENT("Allocate", FColor::Magenta);
		
		for (FUploadedBuffer& UploadedBuffer : UploadedBuffers)
		{
			RDGBuffer* Buffer = UploadedBuffer.Buffer;
			if (!Buffer->ResourceRHI)
			{
				FRDGPooledBufferRef PooledBuffer = AllocatePooledBufferRHI(Buffer);
				Buffer->ResourceRHI = PooledBuffer->GetRHI();
				Buffer->PooledBuffer = PooledBuffer;
			}
		}
	}
	for (FUploadedBuffer& UploadedBuffer : UploadedBuffers)
	{
		QueueBufferUpload(UploadedBuffer.Buffer, UploadedBuffer.Data, UploadedBuffer.DataSize);
		UploadedBuffer.Buffer->bQueuedForUpload = false;
	}
	UploadedBuffers.clear();
}

void RenderGraph::Execute()
{
	RDGPassDesc PresentPassDesc{"PresentPass"};
	PresentPassDesc.bNeverCull = true;
	PresentPass = new FRDGPass(Passes.size(), PresentPassDesc, ERHIPipeline::Graphics);
	FRDGPass::FTextureState& PassState = PresentPass->FindOrAddTextureState(SwapChainTexture);
	PassState.ReferenceCount++;
	SwapChainTexture->ReferenceCount++;
	FRDGTextureSubresourceRange WholeRange = SwapChainTexture->GetSubresourceRange();
	for (FRDGTextureSubresource& Subresource : WholeRange)
	{
		int32 SubresourceIndex = WholeRange.GetSubresourceIndex(Subresource);
		PassState.Access[SubresourceIndex] = ERHIAccess::Present;
	}
	Passes.push_back(PresentPass);

	RDGPassDesc EpiloguePassDesc{"EpiloguePass"};
	EpiloguePassDesc.bNeverCull = true;
	EpiloguePass = new FRDGPass(Passes.size(), EpiloguePassDesc, ERHIPipeline::Graphics);
	Passes.push_back(EpiloguePass);

	const FRDGPassHandle ProloguePassHandle = GetProloguePassHandle();
	const FRDGPassHandle EpiloguePassHandle = GetEpiloguePassHandle();

	const int32 NumBuffers           = Buffers.size();
	const int32 NumTextures          = Textures.size();
	FCollectResourceContext CollectResourceContext;

	SubmitBufferUploads();

    Compile();

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

	CollectPassBarriers();

	CreateViews(CollectResourceContext.Views);
	
	for (FRDGPassHandle PassHandle = ProloguePassHandle+1; PassHandle < EpiloguePassHandle; ++PassHandle)
	{
		FRDGPass* Pass = Passes[PassHandle];

		if (Pass->bCulled)
		{
			RDG_DEBUG_LOG(Display, "Culled pass: {}", Pass->Desc.Name);
			continue;
		}

		RDG_DEBUG_LOG(Display, "Executing pass: {}, Pipeline: {}", Pass->Desc.Name, magic_enum::enum_name(Pass->Pipeline));
		RHICommandList* RHICmdList = nullptr;
		if (Pass->Pipeline == ERHIPipeline::Graphics)
			RHICmdList = RHICreateGfxCommandList();
		else if (Pass->Pipeline == ERHIPipeline::AsyncCompute)
			RHICmdList = RHICreateComputeCommandList();
		else if (Pass->Pipeline == ERHIPipeline::Copy)
			RHICmdList = RHICreateTransferCommandList();
		CollectPassDescriptorSets(PassHandle);
		ExecuteSerialPass(*RHICmdList, Pass);
		RHISubmitCommandList(RHICmdList, Pass->SemaphoresToWait, Pass->SemaphoresToSignal);
	}

}

void RenderGraph::ExecuteSerialPass(RHICommandList& RHICmdList, FRDGPass* Pass)
{
	for (auto& Barrier : Pass->MemoryBarriers)
	{
		RDG_DEBUG_LOG(Display, "[MemoryBarrier] SrcAccess: {}, DstAccess: {}, SrcStage: {}, DstStage: {}", 
			GetAccessName(Barrier.SrcAccess),
			GetAccessName(Barrier.DstAccess),
			GetPipelineStageName(Barrier.SrcStage),
			GetPipelineStageName(Barrier.DstStage));
	}
	for (auto& Barrier : Pass->ImageBarriers)
	{
		RDG_DEBUG_LOG(Display, "[ImageBarrier] RHITexture: 0x{:x}, Subresource: [.MipIndex: {}, .ArraySlice: {}, .PlaneSlice: {}], SrcAccess: {}, DstAccess: {}, SrcStage: {}, DstStage: {}, OldLayout: {}, NewLayout: {}", 
			(size_t)Barrier.Texture, 
			Barrier.Subresource.GetMipIndex(),
			Barrier.Subresource.GetArraySlice(),
			Barrier.Subresource.GetPlaneSlice(),
			GetAccessName(Barrier.SrcAccess), 
			GetAccessName(Barrier.DstAccess),
			GetPipelineStageName(Barrier.SrcStage),
			GetPipelineStageName(Barrier.DstStage),
			GetTextureLayoutName(Barrier.OldLayout),
			GetTextureLayoutName(Barrier.NewLayout));
	}
	for (auto& Barrier : Pass->BufferBarriers)
	{
		RDG_DEBUG_LOG(Display, "[BufferBarrier] RHIBuffer: 0x{:x}, SrcAccess: {}, DstAccess: {}, SrcStage: {}, DstStage: {}, Offset: {}, Size: {}", 
			(size_t)Barrier.Buffer, 
			GetAccessName(Barrier.SrcAccess), 
			GetAccessName(Barrier.DstAccess),
			GetPipelineStageName(Barrier.SrcStage),
			GetPipelineStageName(Barrier.DstStage),
			Barrier.Offset,
			Barrier.Size);
	}
	RHICmdList.PipelineBarrier(Pass->MemoryBarriers, Pass->ImageBarriers, Pass->BufferBarriers);
	if (Pass->Pipeline == ERHIPipeline::Graphics &&
		Pass != PresentPass)
	{
		FRHIRenderPassInfo Info;
		auto& ColorAttachments = Info.ColorRenderTargets;
		auto& DepthStencilAttachment = Info.DepthStencilRenderTarget;
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
		Info.RTLayout = Pass->RenderTargets.GetRenderTargetLayout();
		RHICmdList.BeginRenderPass(Info);
		RHICmdList.SetViewport(Info.Offset.x, Info.Offset.y, Info.Extent.x, Info.Extent.y);
		RHICmdList.SetScissor(Info.Offset.x, Info.Offset.y, Info.Extent.x, Info.Extent.y);
	}
	Pass->Execute(RHICmdList);
	if (Pass->Pipeline == ERHIPipeline::Graphics &&
		Pass != PresentPass)
	{
		RHICmdList.EndRenderPass();
	}
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

	if (!Texture->bTransient)
	{
		PooledTextures.push_back(Texture);
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

	if (!Buffer->bTransient)
	{
		PooledBuffers.push_back(Buffer);
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
		FRDGPass* Pass = Passes[Op.PassHandle];

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			FRDGPooledTextureRef PooledTexture = AllocatePooledRenderTargetRHI(Texture);
			Texture->ResourceRHI = PooledTexture->GetRHI();
			Texture->PooledTexture = PooledTexture;
			RDG_DEBUG_LOG(Display, "[AllocatePooledTexture] Pass: {}, RDGTexture: {} (0x{:x}), RHITexture: 0x{:x}", Pass->GetName(), Texture->Name, (size_t)Texture, (size_t)Texture->ResourceRHI);
			break;
		}
		case FCollectResourceOp::EOp::Deallocate:
			// Texture->PooledTexture = nullptr;
			// Texture->ResourceRHI = nullptr;
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
		FRDGPass* Pass = Passes[Op.PassHandle];

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			FRDGPooledBufferRef PooledBuffer = AllocatePooledBufferRHI(Buffer);
			Buffer->ResourceRHI = PooledBuffer->GetRHI();
			Buffer->PooledBuffer = PooledBuffer;
			RDG_DEBUG_LOG(Display, "[AllocatePooledBuffer] Pass: {}, RDGBuffer: {} (0x{:x}), RHIBuffer: 0x{:x}", Pass->GetName(), Buffer->Name, (size_t)Buffer, (size_t)Buffer->ResourceRHI);
			break;
		}
		case FCollectResourceOp::EOp::Deallocate:
			// Buffer->ResourceRHI = nullptr;
			// Buffer->PooledBuffer = nullptr;
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
		if (Op.Resource->bExternal)
		{
			continue;
		}

		FRDGPass* Pass = Passes[PassHandle];

		switch (Op.Op)
		{
		case FCollectResourceOp::EOp::Allocate:
		{
			if (Op.Resource->Type == ERDGResourceType::Buffer)
			{
				RDGBuffer* Buffer = static_cast<RDGBuffer*>(Op.Resource);
				FRHITransientBuffer* TransientBuffer = TransientResourceAllocator->CreateBuffer(Buffer->Desc.Translate(), Buffer->Name, PassHandle);

				SetTransientBufferRHI(Buffer, TransientBuffer);

				RDG_DEBUG_LOG(Display, "[AllocateTransientBuffer] Pass: {}, RDGBuffer: {} (0x{:x}), RHIBuffer: 0x{:x}", Pass->GetName(), Buffer->Name, (size_t)Buffer, (size_t)Buffer->ResourceRHI);

				Buffer->MinAcquirePass = FRDGPassHandle(TransientBuffer->GetAcquirePasses().Min);
			}
			else
			{
				RDGTexture* Texture = static_cast<RDGTexture*>(Op.Resource);
				FRHITransientTexture* TransientTexture = TransientResourceAllocator->CreateTexture(Texture->Desc, Texture->Name, PassHandle);

                SetTransientTextureRHI(Texture, TransientTexture);

				RDG_DEBUG_LOG(Display, "[AllocateTransientTexture] Pass: {}, RDGTexture: {} (0x{:x}), RHITexture: 0x{:x}", Pass->GetName(), Texture->Name, (size_t)Texture, (size_t)Texture->ResourceRHI);

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

				RDG_DEBUG_LOG(Display, "[DeallocateTransientBuffer] Pass: {}, RDGBuffer: {} (0x{:x}), RHIBuffer: 0x{:x}", Pass->GetName(), Buffer->Name, (size_t)Buffer, (size_t)Buffer->ResourceRHI);

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

				RDG_DEBUG_LOG(Display, "[DeallocateTransientTexture] Pass: {}, RDGTexture: {} (0x{:x}), RHITexture: 0x{:x}", Pass->GetName(), Texture->Name, (size_t)Texture, (size_t)Texture->ResourceRHI);
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

void RenderGraph::CollectPassBarriers(FRDGPassHandle PassHandle)
{
	FRDGPass* CurrentPass = Passes[PassHandle];

	if (CurrentPass->bCulled)
	{
		return;
	}

	const ERHIPipeline CurrentPipeline = CurrentPass->Pipeline;
	RDG_DEBUG_LOG(Display, "[CollectPassBarriers] Pass: {}", CurrentPass->GetName());

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
			ERHIAccess LastAccess = LastState.Access;
			ERHIAccess CurrentAccess = PassState.Access[Index];
			
			if (LastAccess != CurrentAccess)
			{
				const ERHIPipeline LastPipeline = LastState.Pass != NullPassHandle ? Passes[LastState.Pass]->Pipeline : ERHIPipeline::None;
				RHIImageMemoryBarrier Barrier = RHIImageMemoryBarrier(
					Texture->GetRHI(), 
					LastAccess, CurrentAccess, 
					GetPipelineStage(LastPipeline, LastAccess), GetPipelineStage(CurrentPipeline, CurrentAccess),
					GetTextureLayout(LastAccess), GetTextureLayout(CurrentAccess),
					Texture->GetSubresource(Index));
				RDG_DEBUG_LOG(Display, "[CollectPassBarriers New ImageBarrier] RDGTexture: {} (0x{:x}), Subresource: {}, SrcAccess: {}, DstAccess: {}, SrcStage: {}, DstStage: {}, OldLayout: {}, NewLayout: {}", 
					Texture->Name, 
					(size_t)Texture, 
					Index, 
					GetAccessName(Barrier.SrcAccess), 
					GetAccessName(Barrier.DstAccess),
					GetPipelineStageName(Barrier.SrcStage),
					GetPipelineStageName(Barrier.DstStage),
					GetTextureLayoutName(Barrier.OldLayout),
					GetTextureLayoutName(Barrier.NewLayout));
				CurrentPass->ImageBarriers.push_back(Barrier);
				if (LastState.Pass != NullPassHandle)
				{
					FRDGPass* LastPass = Passes[LastState.Pass];
					RDG_DEBUG_LOG(Display, "[CollectPassBarriers New ImageBarrier] LastPass: {}, CurrentPass: {}", LastPass->GetName(), CurrentPass->GetName());
					if (LastPass->Pipeline != CurrentPipeline)
					{
						RHISemaphoreRef Semaphore = RHICreateSemaphore();
						LastPass->SemaphoresToSignal.push_back(Semaphore);
						CurrentPass->SemaphoresToWait.push_back(Semaphore);
					}
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
		ERHIAccess LastAccess = LastState.Access;
		ERHIAccess CurrentAccess = PassState.Access;

		if (LastAccess != CurrentAccess)
		{
			const ERHIPipeline LastPipeline = LastState.Pass != NullPassHandle ? Passes[LastState.Pass]->Pipeline : ERHIPipeline::None;
			RHIBufferMemoryBarrier Barrier = RHIBufferMemoryBarrier(
				Buffer->GetRHI(), 
				LastAccess, CurrentAccess, 
				GetPipelineStage(LastPipeline, LastAccess), GetPipelineStage(CurrentPipeline, CurrentAccess),
				0, Buffer->Desc.GetSize());
			RDG_DEBUG_LOG(Display, "\tRDGBuffer: {} (0x{:x}), SrcAccess: {}, DstAccess: {}, SrcStage: {}, DstStage: {}, Offset: {}, Size: {}", 
				Buffer->Name, 
				(size_t)Buffer, 
				GetAccessName(Barrier.SrcAccess), 
				GetAccessName(Barrier.DstAccess), 
				GetPipelineStageName(Barrier.SrcStage), 
				GetPipelineStageName(Barrier.DstStage), 
				Barrier.Offset, Barrier.Size);
			CurrentPass->BufferBarriers.push_back(Barrier);
			if (LastState.Pass != NullPassHandle)
			{
				FRDGPass* LastPass = Passes[LastState.Pass];
				RDG_DEBUG_LOG(Display, "\tLastPass: {}, CurrentPass: {}", LastPass->GetName(), CurrentPass->GetName());
				if (LastPass->Pipeline != CurrentPipeline)
				{
					RHISemaphoreRef Semaphore = RHICreateSemaphore();
					LastPass->SemaphoresToSignal.push_back(Semaphore);
					CurrentPass->SemaphoresToWait.push_back(Semaphore);
				}
			}
		}

		LastState.Access = CurrentAccess;
		LastState.Pass = CurrentPass->Handle;
	}
}

void RenderGraph::CollectPassDescriptorSets(FRDGPassHandle PassHandle)
{
	FRDGPass* CurrentPass = Passes[PassHandle];

	if (CurrentPass->bCulled)
	{
		return;
	}

	RDG_DEBUG_LOG(Display, "Collecting descriptor sets for pass: {}", CurrentPass->Desc.Name);

	for (RDGDescriptorSet* DescriptorSet : CurrentPass->DescriptorSets)
	{
		RHIDescriptorSetLayout* Layout = DescriptorSet->GetLayout();
		RHIDescriptorSetPools& Pool = DescriptorSetPools.try_emplace(Layout, Layout).first->second;
		RHIDescriptorSet* DescriptorSetRHI = Pool.Allocate();
		DescriptorSet->ResourceRHI = DescriptorSetRHI;

		RDG_DEBUG_LOG(Display, "Allocated descriptor set: {} 0x{:x}, RHIResource: 0x{:x}", DescriptorSet->Name, (size_t)DescriptorSet, (size_t)DescriptorSetRHI);

		// check every binding is updated
		for (auto& Binding : Layout->Bindings)
		{
			uint32 BindingIndex = Binding.BindingIndex;
			EDescriptorType DescriptorType = Binding.DescriptorType;
			auto& WriterInfo = DescriptorSet->WriterInfos.at(BindingIndex);
			Ncheckf(DescriptorType == WriterInfo.DescriptorType, "Descriptor type mismatch");
			RDG_DEBUG_LOG(Display, "Descriptor set {} Binding {} updated, DescriptorType: {}, RHIResource: 0x{:x}", 
				DescriptorSet->Name, 
				BindingIndex, 
				magic_enum::enum_name(WriterInfo.DescriptorType), 
				(size_t)DescriptorSetRHI);
			switch (WriterInfo.DescriptorType)
			{
			case EDescriptorType::UniformBuffer:
			{
				auto& BufferInfo = WriterInfo.BufferInfo;
				DescriptorSetRHI->SetUniformBuffer(BindingIndex, BufferInfo.Buffer->GetRHI());
				break;
			}
			case EDescriptorType::StorageBuffer:
			{
				auto& BufferInfo = WriterInfo.BufferInfo;
				DescriptorSetRHI->SetStorageBuffer(BindingIndex, BufferInfo.Buffer->GetRHI());
				break;
			}
			case EDescriptorType::CombinedImageSampler:
			{
				auto& ImageInfo = WriterInfo.ImageInfo;
				DescriptorSetRHI->SetSampler(BindingIndex, ImageInfo.Texture->GetRHI(), ImageInfo.SamplerState);
				break;
			}
			case EDescriptorType::StorageImage:
			{
				auto& ImageInfo = WriterInfo.ImageInfo;
				DescriptorSetRHI->SetStorageImage(BindingIndex, ImageInfo.Texture->GetRHI());
				break;
			}
			default:
			{
				Ncheckf(false, "Unknown descriptor type {}", magic_enum::enum_name(WriterInfo.DescriptorType));
				break;
			}
			}
		}
	}
}

}