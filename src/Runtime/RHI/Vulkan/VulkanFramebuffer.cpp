#include "VulkanFramebuffer.h"
#include "VulkanDynamicRHI.h"
#include "VulkanTexture.h"

namespace nilou {

VulkanFramebuffer::VulkanFramebuffer(FVulkanDynamicRHI* InContext, std::map<EFramebufferAttachment, RHITexture2DRef> InAttachments)
    : Context(InContext)
{
    Attachments = InAttachments;
    std::vector<VkImageView> attachments;
    VkFramebufferCreateInfo framebufferInfo{};
    for (auto& [Attachment, Texture] : InAttachments)
    {
        VulkanTexture2D* vkTexture = static_cast<VulkanTexture2D*>(Texture.get());
        attachments.push_back(vkTexture->GetImageView());
        framebufferInfo.width = Texture->GetSizeX();
        framebufferInfo.height = Texture->GetSizeY();
    }
    FGraphicsPipelineStateInitializer Initializer;
    Initializer.BuildRenderTargetFormats(this);
    FVulkanRenderTargetLayout RTLayout(Initializer);
    FVulkanRenderPass* RenderPass = Context->RenderPassManager->GetOrCreateRenderPass(RTLayout);
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = RenderPass->Handle;
    framebufferInfo.attachmentCount = static_cast<uint32>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(Context->device, &framebufferInfo, nullptr, &Handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    vkDestroyFramebuffer(Context->device, Handle, nullptr);
}

RHIFramebufferRef FVulkanDynamicRHI::RHICreateFramebuffer(std::map<EFramebufferAttachment, RHITexture2DRef> Attachments)
{
    VulkanFramebufferRef Framebuffer = std::make_shared<VulkanFramebuffer>(this, Attachments);
    return Framebuffer;
}

}