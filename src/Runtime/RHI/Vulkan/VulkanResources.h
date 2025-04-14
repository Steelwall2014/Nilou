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

class VulkanGraphicsPipelineState : public RHIGraphicsPipelineState
{
public:
    VulkanGraphicsPipelineState(VkDevice InDevice, const FGraphicsPipelineStateInitializer& InInInitializer)
        : Device(InDevice)
        , RHIGraphicsPipelineState(InInInitializer)
    { }
    VkDevice Device{};
    VkPipeline Handle{};
    VkRenderPass RenderPass{};
    ~VulkanGraphicsPipelineState();
};
using VulkanGraphicsPipelineStateRef = TRefCountPtr<VulkanGraphicsPipelineState>;
inline VulkanGraphicsPipelineState* ResourceCast(RHIGraphicsPipelineState* PSO)
{
    return static_cast<VulkanGraphicsPipelineState*>(PSO);
}

class VulkanComputePipelineState : public RHIComputePipelineState
{
public:
    VulkanComputePipelineState(VkDevice InDevice, RHIComputeShader* ComputeShader)
        : Device(InDevice)
        , RHIComputePipelineState(ComputeShader)
    { }
    VkDevice Device{};
    VkPipeline Handle{};
    ~VulkanComputePipelineState()
    {
        vkDestroyPipeline(Device, Handle, nullptr);
    }
};
using VulkanComputePipelineStateRef = TRefCountPtr<VulkanComputePipelineState>;
inline VulkanComputePipelineState* ResourceCast(RHIComputePipelineState* PSO)
{
    return static_cast<VulkanComputePipelineState*>(PSO);
}

class VulkanDepthStencilState : public RHIDepthStencilState
{
public:
    VulkanDepthStencilState(const FDepthStencilStateInitializer& Initializer);
    VkPipelineDepthStencilStateCreateInfo DepthStencilState{};
};
using VulkanDepthStencilStateRef = TRefCountPtr<VulkanDepthStencilState>;

class VulkanRasterizerState : public RHIRasterizerState
{
public:
    VulkanRasterizerState(const FRasterizerStateInitializer& Initializer);
    VkPipelineRasterizationStateCreateInfo RasterizerState{};
};
using VulkanRasterizerStateRef = TRefCountPtr<VulkanRasterizerState>;

class VulkanBlendState : public RHIBlendState
{
public:
    VulkanBlendState(const FBlendStateInitializer& Initializer);
	VkPipelineColorBlendAttachmentState BlendStates[MaxSimultaneousRenderTargets];
};
using VulkanBlendStateRef = TRefCountPtr<VulkanBlendState>;

class VulkanSamplerState : public RHISamplerState
{
public:
    VulkanSamplerState(const FSamplerStateInitializer& InInitializer, VkDevice InDevice) 
        : Device(InDevice) 
        , RHISamplerState(InInitializer) { }
    ~VulkanSamplerState()
    {
        vkDestroySampler(Device, Handle, nullptr);
    }
    VkDevice Device{};
    VkSampler Handle{};
};
using VulkanSamplerStateRef = TRefCountPtr<VulkanSamplerState>;

inline VulkanSamplerState* ResourceCast(RHISamplerState* Sampler)
{
    return static_cast<VulkanSamplerState*>(Sampler);
}

// struct FVulkanRenderTargetLayout
// {
//     FVulkanRenderTargetLayout(const RHIRenderTargetLayout& RTLayout);
    
//     std::vector<VkAttachmentDescription> Desc;
//     std::vector<VkAttachmentReference> ColorReferences;
//     VkAttachmentReference DepthStencilReference{};

//     uint32 RenderPassFullHash = 0;

//     bool bHasDepthAttachment = false;

//     bool operator==(const FVulkanRenderTargetLayout& Other) const
//     {
//         if (Desc.size() != Other.Desc.size() || ColorReferences.size() != Other.ColorReferences.size())
//             return false;
//         size_t desc_size = sizeof(VkAttachmentDescription) * Desc.size();
//         size_t ref_size = sizeof(VkAttachmentReference) * ColorReferences.size();
//         if (std::memcmp(Desc.data(), Other.Desc.data(), desc_size) != 0 || 
//             std::memcmp(ColorReferences.data(), Other.ColorReferences.data(), ref_size) != 0 ||
//             std::memcmp(&DepthStencilReference, &Other.DepthStencilReference, sizeof(VkAttachmentReference)) != 0)
//             return false;

//         return true;
//     }

// private:
//     void InitWithAttachments(
//         const std::array<EPixelFormat, MaxSimultaneousRenderTargets>& RenderTargetFormats,
//         EPixelFormat DepthStencilTargetFormat);

// };

}

namespace std {

// template<>
// struct hash<nilou::FVulkanRenderTargetLayout>
// {
//     size_t operator()(const nilou::FVulkanRenderTargetLayout& _Keyval) const noexcept
//     {
//         return _Keyval.RenderPassFullHash;
//     }
// };

template<>
struct hash<nilou::RHIRenderTargetLayout>
{
    size_t operator()(const nilou::RHIRenderTargetLayout& _Keyval) const noexcept;
};

}

namespace nilou {

struct FVulkanRenderPassManager
{
    FVulkanRenderPassManager(VkDevice InDevice)
        : Device(InDevice)
    { }
    ~FVulkanRenderPassManager();
    VkDevice Device{};
    VkFramebuffer GetOrCreateFramebuffer(VkRenderPass RenderPass, const std::array<RHITextureView*, MaxSimultaneousRenderTargets>& ColorAttachments, RHITextureView* DepthStencilAttachment);
    VkRenderPass GetOrCreateRenderPass(const RHIRenderTargetLayout& RTLayout);
    std::unordered_map<uint32, VkRenderPass> RenderPasses;
    std::unordered_map<uint32, VkFramebuffer> Framebuffers;
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