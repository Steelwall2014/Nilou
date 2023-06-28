#pragma once
#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"
#include "Platform.h"

namespace nilou {

struct FVulkanDescriptorSetWriter
{
    std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo> DescriptorInfo;
    VkWriteDescriptorSet WriteDescriptor;
};

class FVulkanCommonPipelineDescriptorState
{
public:

    void SetUniformBuffer(uint8 BindingIndex, VulkanUniformBuffer* Buffer);

    void SetSampler(uint8 BindingIndex, FRHISampler Sampler);

    std::unordered_map<uint8, FVulkanDescriptorSetWriter> Writers;
};

}