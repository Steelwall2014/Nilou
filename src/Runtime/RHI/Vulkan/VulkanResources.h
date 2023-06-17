#pragma once
#include "VulkanShader.h"

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

}