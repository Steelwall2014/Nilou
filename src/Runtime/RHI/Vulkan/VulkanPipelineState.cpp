#include "VulkanPipelineState.h"
#include "VulkanTexture.h"
#include "VulkanResources.h"
#include "VulkanDynamicRHI.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"

namespace nilou {

// void FVulkanCommonPipelineDescriptorState::SetUniformBuffer(uint8 BindingIndex, RHIUniformBuffer* Buffer)
// {
//     VulkanUniformBuffer* vkBuffer = static_cast<VulkanUniformBuffer*>(Buffer);
//     FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
//     VkDescriptorBufferInfo& Info = Writer.BufferInfo;//std::get<VkDescriptorBufferInfo>(Writer.DescriptorInfo);
//     Info.buffer = vkBuffer->GetHandle();
//     Info.offset = 0;
//     Info.range = vkBuffer->GetSize();
//     Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
//     Writer.WriteDescriptor.dstBinding = BindingIndex;
//     Writer.WriteDescriptor.dstArrayElement = 0;
//     Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     Writer.WriteDescriptor.descriptorCount = 1;
//     Writer.WriteDescriptor.pBufferInfo = &Info;
//     //vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
// }

// void FVulkanCommonPipelineDescriptorState::SetBuffer(uint8 BindingIndex, RHIBuffer* Buffer)
// {
//     VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(Buffer);
//     FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
//     VkDescriptorBufferInfo& Info = Writer.BufferInfo;//std::get<VkDescriptorBufferInfo>(Writer.DescriptorInfo);
//     Info.buffer = vkBuffer->GetHandle();
//     Info.offset = 0;
//     Info.range = vkBuffer->GetSize();
//     Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
//     Writer.WriteDescriptor.dstBinding = BindingIndex;
//     Writer.WriteDescriptor.dstArrayElement = 0;
//     Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//     Writer.WriteDescriptor.descriptorCount = 1;
//     Writer.WriteDescriptor.pBufferInfo = &Info;
//     //vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
// }

// void FVulkanCommonPipelineDescriptorState::SetSampler(uint8 BindingIndex, FRHISampler Sampler)
// {
// 	VulkanTexture* vkTexture = ResourceCast(Sampler.Texture);
// 	{
// 		Barrier.AddImageLayoutTransition(
// 			vkTexture, GetFullAspectMask(Sampler.Texture->GetFormat()),
// 			vkTexture->GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
// 	}
//     VulkanSamplerState* vkSampler = static_cast<VulkanSamplerState*>(Sampler.SamplerState);
//     FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
//     VkDescriptorImageInfo& Info = Writer.ImageInfo;//std::get<VkDescriptorImageInfo>(Writer.ImageInfo);
//     Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//     Info.imageView = vkTexture->ImageView;
//     Info.sampler = vkSampler->Handle;
//     Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
//     Writer.WriteDescriptor.dstBinding = BindingIndex;
//     Writer.WriteDescriptor.dstArrayElement = 0;
//     Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     Writer.WriteDescriptor.descriptorCount = 1;
//     Writer.WriteDescriptor.pImageInfo = &Info;
//     //vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
// }

// void FVulkanCommonPipelineDescriptorState::SetImage(uint8 BindingIndex, RHITexture* Image, EDataAccessFlag Access)
// {
//     VulkanTexture* vkTexture = ResourceCast(Image);
// 	{
// 		Barrier.AddImageLayoutTransition(
// 			vkTexture, GetFullAspectMask(Image->GetFormat()),
// 			vkTexture->GetImageLayout(), VK_IMAGE_LAYOUT_GENERAL);
// 	}
//     FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
//     VkDescriptorImageInfo& Info = Writer.ImageInfo;//std::get<VkDescriptorImageInfo>(Writer.DescriptorInfo);
// 	Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//     Info.imageView = vkTexture->ImageView;
//     Info.sampler = VK_NULL_HANDLE;
//     Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
//     Writer.WriteDescriptor.dstBinding = BindingIndex;
//     Writer.WriteDescriptor.dstArrayElement = 0;
//     Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ;
//     Writer.WriteDescriptor.descriptorCount = 1;
//     Writer.WriteDescriptor.pImageInfo = &Info;
//     //vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
// }

// void FVulkanCommonPipelineDescriptorState::UpdateDescriptorSet()
// {
// 	Barrier.Execute(Context->CommandBufferManager->GetUploadCmdBuffer());
// 	Context->CommandBufferManager->SubmitUploadCmdBuffer();
// 	std::vector<VkWriteDescriptorSet> WriteInfo;
// 	WriteInfo.reserve(Writers.size());
// 	for (auto& [Binding, Writer] : Writers)
// 	{
// 		WriteInfo.push_back(Writer.WriteDescriptor);
// 	}
//     vkUpdateDescriptorSets(Context->device, WriteInfo.size(), WriteInfo.data(), 0, nullptr);
// }

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	vkDestroyPipelineLayout(Device, Handle, nullptr);
	for (int i = 0; i < DescriptorSetsLayout->Handles.size(); i++)
		vkDestroyDescriptorSetLayout(Device, DescriptorSetsLayout->Handles[i], nullptr);
}

}