#include "VulkanFramebuffer.h"
#include "VulkanDynamicRHI.h"
#include "VulkanTexture.h"
#include "Common/Containers/Array.h"

namespace nilou {

// VulkanFramebuffer::VulkanFramebuffer(FVulkanDynamicRHI* InContext, const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& InAttachments, RHITextureView* InDepthStencilAttachment)
//     : Context(InContext)
// {
//     Attachments = InAttachments;
//     std::vector<VkImageView> attachments;
//     VkFramebufferCreateInfo framebufferInfo{};
//     RHIRenderTargetLayout RTLayout;
//     for (auto& [Attachment, Texture] : Enumerate(InAttachments))
//     {
//         if (Texture)
//         {
//             VulkanTextureView* vkTexture = ResourceCast(Texture);
//             attachments.push_back(vkTexture->GetHandle());
//             framebufferInfo.width = Texture->GetSizeX();
//             framebufferInfo.height = Texture->GetSizeY();
//             RTLayout.RenderTargetFormats[Attachment] = Texture->Desc.Format;
//         }
//     }
//     FVulkanRenderPass* RenderPass = Context->RenderPassManager->GetOrCreateRenderPass(RTLayout);
//     framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//     framebufferInfo.renderPass = RenderPass->Handle;
//     Ncheck(attachments.size() != 0);
//     framebufferInfo.attachmentCount = static_cast<uint32>(attachments.size());
//     framebufferInfo.pAttachments = attachments.data();
//     framebufferInfo.layers = 1;

//     if (vkCreateFramebuffer(Context->device, &framebufferInfo, nullptr, &Handle) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create framebuffer!");
//     }
// }

// VulkanFramebuffer::~VulkanFramebuffer()
// {
//     vkDestroyFramebuffer(Context->device, Handle, nullptr);
// }

// RHIFramebufferRef FVulkanDynamicRHI::RHICreateFramebuffer(const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& InAttachments, RHITextureView* InDepthStencilAttachment)
// {
//     VulkanFramebufferRef Framebuffer = new VulkanFramebuffer(this, InAttachments, InDepthStencilAttachment);
//     return Framebuffer;
// }

}