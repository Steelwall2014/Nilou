#pragma once
#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
#include "Platform.h"
#include "VulkanDescriptorSet.h"
#include "VulkanBarriers.h"

namespace nilou {

// struct FVulkanDescriptorSetWriter
// {
//     VkDescriptorBufferInfo BufferInfo{};
//     VkDescriptorImageInfo ImageInfo{};
//     VkWriteDescriptorSet WriteDescriptor;
// };

// class FVulkanCommonPipelineDescriptorState
// {
// public:

//     FVulkanCommonPipelineDescriptorState(FVulkanDynamicRHI* InContext, FVulkanDescriptorSets InDescriptorSets)
//         : Context(InContext)
//         , DescriptorSets(InDescriptorSets)
//     { }

//     void SetUniformBuffer(uint8 BindingIndex, RHIUniformBuffer* Buffer);

//     void SetBuffer(uint8 BindingIndex, RHIBuffer* Buffer);

//     void SetSampler(uint8 BindingIndex, FRHISampler Sampler);

//     void SetImage(uint8 BindingIndex, RHITexture* Image, EDataAccessFlag Access);

//     void UpdateDescriptorSet();

//     std::unordered_map<uint8, FVulkanDescriptorSetWriter> Writers;

//     FVulkanDynamicRHI* Context;

//     FVulkanDescriptorSets DescriptorSets;

//     FVulkanImageLayoutBarrierHelper Barrier;
// };

class VulkanPipelineLayout : public RHIPipelineLayout
{
public:
    VulkanPipelineLayout(VkDevice InDevice)
        : Device(InDevice)
    {}
    VkDevice Device;
    std::vector<VkDescriptorSetLayout> SetLayoutHandles;
    VkPipelineLayout Handle;
    ~VulkanPipelineLayout();
};
using VulkanPipelineLayoutRef = TRefCountPtr<VulkanPipelineLayout>;

inline VulkanPipelineLayout* ResourceCast(RHIPipelineLayout* PipelineLayout)
{
    return static_cast<VulkanPipelineLayout*>(PipelineLayout);
}

}