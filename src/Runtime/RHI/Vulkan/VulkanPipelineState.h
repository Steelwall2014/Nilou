#pragma once
#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
#include "Platform.h"
#include "VulkanDescriptorSet.h"

namespace nilou {

struct FVulkanDescriptorSetWriter
{
    union
    {
        VkDescriptorBufferInfo BufferInfo;
        VkDescriptorImageInfo ImageInfo;
    };
    //std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo> DescriptorInfo;
    VkWriteDescriptorSet WriteDescriptor;
};

class FVulkanCommonPipelineDescriptorState
{
public:

    FVulkanCommonPipelineDescriptorState(FVulkanDynamicRHI* InContext, FVulkanDescriptorSets InDescriptorSets)
        : Context(InContext)
        , DescriptorSets(InDescriptorSets)
    { }

    void SetUniformBuffer(uint8 BindingIndex, RHIUniformBuffer* Buffer);

    void SetBuffer(uint8 BindingIndex, RHIBuffer* Buffer);

    void SetSampler(uint8 BindingIndex, FRHISampler Sampler);

    void SetImage(uint8 BindingIndex, RHITexture* Image, EDataAccessFlag Access);

    std::unordered_map<uint8, FVulkanDescriptorSetWriter> Writers;

    FVulkanDynamicRHI* Context;

    FVulkanDescriptorSets DescriptorSets;
};

class VulkanPipelineLayout : public FRHIPipelineLayout
{
public:
    VulkanPipelineLayout(VkDevice InDevice)
        : Device(InDevice)
    {}
    VkDevice Device;
    std::shared_ptr<FVulkanDescriptorSetsLayout> DescriptorSetsLayout;
    VkPipelineLayout PipelineLayout;
    ~VulkanPipelineLayout();
};
using VulkanPipelineLayoutRef = std::shared_ptr<VulkanPipelineLayout>;

}