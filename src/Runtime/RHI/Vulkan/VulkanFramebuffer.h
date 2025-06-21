#pragma once

#include <vulkan/vulkan.h>
#include "VulkanResources.h"

namespace nilou {

// class FVulkanDynamicRHI;

// class VulkanFramebuffer : public RHIFramebuffer
// {
// public:
//     VkFramebuffer Handle{};
//     FVulkanDynamicRHI* Context;
//     virtual bool Check() override { return Handle != VK_NULL_HANDLE; }
//     virtual ~VulkanFramebuffer();
//     VulkanFramebuffer(FVulkanDynamicRHI* InContext, const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& InAttachments, RHITextureView* InDepthStencilAttachment);

// };
// using VulkanFramebufferRef = TRefCountPtr<VulkanFramebuffer>;

}