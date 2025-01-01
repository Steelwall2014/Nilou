#include "RenderGraphResources.h"
#include "Common/Crc.h"
#include "DynamicRHI.h"
#include "RenderingThread.h"

namespace nilou {

RDGTexture::RDGTexture(std::string InName, const RDGTextureDesc& InDesc)
	: RDGResource(InName, ERDGResourceType::Texture)
	, Desc(InDesc) 
	, Layout(InDesc)
	, WholeRange(Layout)
	, SubresourceCount(Layout.GetSubresourceCount())
{ 
	SubresourceStates.resize(SubresourceCount);
}

void RDGBuffer::Flush()
{
	Ncheck(IsInRenderingThread());
	if (RHIBuffer* BufferRHI = GetRHI())
	{
		void* data = RHIMapMemory(BufferRHI, 0, Desc.GetSize());
			memcpy(data, Data.get(), Desc.GetSize());
		RHIUnmapMemory(BufferRHI);
		bDirty = false;
	}
}

void RDGFramebuffer::SetAttachment(EFramebufferAttachment Attachment, RDGTextureView* Texture)
{
	if (Attachments.find(Attachment) == Attachments.end())
	{
		RTLayout.NumRenderTargetsEnabled += 1;
	}
	Attachments[Attachment] = Texture;
	if (Attachment == FA_Depth_Stencil_Attachment)
	{
		RTLayout.DepthStencilTargetFormat = Texture->Desc.Format;
	}
	else
	{
		RTLayout.RenderTargetFormats[Attachment] = Texture->Desc.Format;
	}
}

uint32 FRDGPooledTexture::ComputeMemorySize() const
{
	uint32 Size = RHIComputeMemorySize(Texture.GetReference());
	return Size;
}

}