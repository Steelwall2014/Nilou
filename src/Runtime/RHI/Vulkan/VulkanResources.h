#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "VulkanSemaphore.h"

namespace nilou {

inline std::shared_ptr<FVulkanSemaphore> CreateSemephore(VkDevice Device)
{
    return std::make_shared<FVulkanSemaphore>(Device);
}

struct FVulkanRenderPass
{
    FVulkanRenderPass(VkDevice InDevice)
        : Device(InDevice)
    { }
    VkDevice Device{};
    VkRenderPass Handle{};
    ~FVulkanRenderPass()
    {
        vkDestroyRenderPass(Device, Handle, nullptr);
    }
};

class VulkanGraphicsPipelineState : public FRHIGraphicsPipelineState
{
public:
    VulkanGraphicsPipelineState(VkDevice InDevice, const FGraphicsPipelineStateInitializer& InInInitializer)
        : Device(InDevice)
        , FRHIGraphicsPipelineState(InInInitializer)
    { }
    VkDevice Device{};
    VkPipeline VulkanPipeline{};
    FVulkanRenderPass* RenderPass{};
    ~VulkanGraphicsPipelineState();
};
using VulkanGraphicsPipelineStateRef = std::shared_ptr<VulkanGraphicsPipelineState>;

class VulkanDepthStencilState : public RHIDepthStencilState
{
public:
    VulkanDepthStencilState(const FDepthStencilStateInitializer& Initializer);
    VkPipelineDepthStencilStateCreateInfo DepthStencilState{};
};
using VulkanDepthStencilStateRef = std::shared_ptr<VulkanDepthStencilState>;

class VulkanRasterizerState : public RHIRasterizerState
{
public:
    VulkanRasterizerState(const FRasterizerStateInitializer& Initializer);
    VkPipelineRasterizationStateCreateInfo RasterizerState{};
};
using VulkanRasterizerStateRef = std::shared_ptr<VulkanRasterizerState>;

class VulkanBlendState : public RHIBlendState
{
public:
    VulkanBlendState(const FBlendStateInitializer& Initializer);
	VkPipelineColorBlendAttachmentState BlendStates[MAX_SIMULTANEOUS_RENDERTARGETS];
};
using VulkanBlendStateRef = std::shared_ptr<VulkanBlendState>;

class VulkanSamplerState : public RHISamplerState
{
public:
    VulkanSamplerState(VkDevice InDevice) : Device(InDevice) { }
    ~VulkanSamplerState()
    {
        vkDestroySampler(Device, Handle, nullptr);
    }
    VkDevice Device{};
    VkSampler Handle{};
};
using VulkanSamplerStateRef = std::shared_ptr<VulkanSamplerState>;

struct FVulkanRenderTargetLayout
{
    FVulkanRenderTargetLayout(const std::unordered_map<EFramebufferAttachment, EPixelFormat>& Attachments);
    FVulkanRenderTargetLayout(const FGraphicsPipelineStateInitializer& Initializer);
    FVulkanRenderTargetLayout(const FRHIRenderPassInfo& Info);
    
    std::vector<VkAttachmentDescription> Desc;
    std::vector<VkAttachmentReference> ColorReferences;
    VkAttachmentReference DepthStencilReference{};

    uint32 RenderPassFullHash = 0;

    bool bHasDepthAttachment = false;

    bool operator==(const FVulkanRenderTargetLayout& Other) const
    {
        if (Desc.size() != Other.Desc.size() || ColorReferences.size() != Other.ColorReferences.size())
            return false;
        size_t desc_size = sizeof(VkAttachmentDescription) * Desc.size();
        size_t ref_size = sizeof(VkAttachmentReference) * ColorReferences.size();
        if (std::memcmp(Desc.data(), Other.Desc.data(), desc_size) != 0 || 
            std::memcmp(ColorReferences.data(), Other.ColorReferences.data(), ref_size) != 0 ||
            std::memcmp(&DepthStencilReference, &Other.DepthStencilReference, sizeof(VkAttachmentReference)) != 0)
            return false;

        return true;
    }

private:
    void InitWithAttachments(
        const std::array<EPixelFormat, MAX_SIMULTANEOUS_RENDERTARGETS>& RenderTargetFormats,
        uint32 NumRenderTargetsEnabled,
        EPixelFormat DepthStencilTargetFormat);

};

}

namespace std {

template<>
struct hash<nilou::FVulkanRenderTargetLayout>
{
    size_t operator()(const nilou::FVulkanRenderTargetLayout& _Keyval) const noexcept
    {
        return _Keyval.RenderPassFullHash;
    }
};

}

namespace nilou {

struct FVulkanRenderPassManager
{
    FVulkanRenderPassManager(VkDevice InDevice)
        : Device(InDevice)
    { }
    VkDevice Device{};
    FVulkanRenderPass* GetOrCreateRenderPass(const FVulkanRenderTargetLayout& RTLayout);
    std::unordered_map<FVulkanRenderTargetLayout, FVulkanRenderPass> RenderPasses;
};

VkFormat TranslatePixelFormatToVKFormat(EPixelFormat Format);

inline VkImageAspectFlags GetAspectMaskFromPixelFormat(EPixelFormat Format, bool bIncludeStencil, bool bIncludeDepth = true)
{
    switch (Format)
    {
    case PF_D24S8:
    case PF_D32FS8:
        return (bIncludeDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) | (bIncludeStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    case PF_D32F:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

inline VkImageAspectFlags GetFullAspectMask(EPixelFormat Format)
{
    return GetAspectMaskFromPixelFormat(Format, true, true);
}

}