#include "RenderGraphResources.h"
#include "Common/Crc.h"
#include "DynamicRHI.h"
#include "Common/Containers/Array.h"
#include "RenderingThread.h"
#include "RenderGraph.h"

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

RHIRenderTargetLayout RDGRenderTargets::GetRenderTargetLayout() const
{
	RHIRenderTargetLayout RTLayout;
	for (auto [i, ColorAttachment] : Enumerate(ColorAttachments))
	{
		if (ColorAttachment.TextureView != nullptr)
		{
			RTLayout.ColorAttachments[i].Format = ColorAttachment.TextureView->Desc.Format;
			RTLayout.ColorAttachments[i].LoadAction = ColorAttachment.LoadAction;
			RTLayout.ColorAttachments[i].StoreAction = ColorAttachment.StoreAction;
		}
	}
	if (DepthStencilAttachment.TextureView != nullptr)
	{
		RTLayout.DepthStencilAttachment.Format = DepthStencilAttachment.TextureView->Desc.Format;
		RTLayout.DepthStencilAttachment.LoadAction = DepthStencilAttachment.LoadAction;
		RTLayout.DepthStencilAttachment.StoreAction = DepthStencilAttachment.StoreAction;
	}
	return RTLayout;
}

uint32 FRDGPooledTexture::ComputeMemorySize() const
{
	uint32 Size = RHIComputeMemorySize(Texture.GetReference());
	return Size;
}

}