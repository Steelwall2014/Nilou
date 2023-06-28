#include "VulkanPipelineState.h"

namespace nilou {

void FVulkanCommonPipelineDescriptorState::SetUniformBuffer(uint8 BindingIndex, VulkanUniformBuffer* Buffer)
{
    // FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
    // VkDescriptorBufferInfo& Info = std::get<VkDescriptorBufferInfo>(Writer.DescriptorInfo);
    // Info.buffer = Buffer->GetHandle();
    // Info.offset = 0;
    // Info.range = Buffer->GetSize();
    // Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // Writer.WriteDescriptor.dstSet = descriptorSets[i];
    // Writer.WriteDescriptor.dstBinding = BindingIndex;
    // Writer.WriteDescriptor.dstArrayElement = 0;
    // Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // Writer.WriteDescriptor.descriptorCount = 1;
    // Writer.WriteDescriptor.pBufferInfo = &Info;
}

}