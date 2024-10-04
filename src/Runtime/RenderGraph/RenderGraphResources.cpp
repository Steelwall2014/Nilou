#include "RenderGraphResources.h"
#include "Common/Crc.h"
#include "DynamicRHI.h"


namespace std {

size_t hash<nilou::RDGTextureDesc>::operator()(const nilou::RDGTextureDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

size_t hash<nilou::RDGBufferDesc>::operator()(const nilou::RDGBufferDesc &_Keyval) const noexcept {
	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
}

// size_t hash<nilou::RDGUniformBufferDesc>::operator()(const nilou::RDGUniformBufferDesc &_Keyval) const noexcept {
// 	return FCrc::MemCrc32(&_Keyval, sizeof(_Keyval));
// }

}

namespace nilou {

void RDGBuffer::Flush()
{
	if (RHIBuffer* BufferRHI = GetRHI())
	{
		void* data = RHIMapMemory(BufferRHI, 0, Desc.Size);
			memcpy(data, Data.get(), Desc.Size);
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

}