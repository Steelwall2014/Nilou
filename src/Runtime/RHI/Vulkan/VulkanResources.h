#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {

struct FVulkanRenderPass
{
    VkRenderPass Handle;
};

class VulkanGraphicsPipelineState : public FRHIGraphicsPipelineState
{
public:
    VkPipeline VulkanPipeline;
    FVulkanRenderPass* RenderPass;
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

class VulkanPipelineLayout : public FRHIPipelineLayout
{
public:
    VkDescriptorSetLayout DescriptorSetLayout;
    VkPipelineLayout PipelineLayout;
};
using VulkanPipelineLayoutRef = std::shared_ptr<VulkanPipelineLayout>;

struct FVulkanRenderTargetLayout
{
    FVulkanRenderTargetLayout(const FGraphicsPipelineStateInitializer& Initializer);
    FVulkanRenderTargetLayout(const FRHIRenderPassInfo& Info);
    
    std::vector<VkAttachmentDescription> Desc;
    std::vector<VkAttachmentReference> ColorReferences;
    VkAttachmentReference DepthStencilReference;

    uint32 RenderPassFullHash;

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
    void InitWithInitializer(const FGraphicsPipelineStateInitializer& Initializer);

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

struct FVulkanLayoutManager
{
    FVulkanLayoutManager(VkDevice InDevice)
        : Device(InDevice)
    { }
    VkDevice Device;
    FVulkanRenderPass* GetOrCreateRenderPass(const FVulkanRenderTargetLayout& RTLayout);
    std::unordered_map<FVulkanRenderTargetLayout, FVulkanRenderPass> RenderPasses;
};

}