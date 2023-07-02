#include "VulkanPipelineState.h"
#include "VulkanTexture.h"
#include "VulkanResources.h"
#include "VulkanDynamicRHI.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"

namespace nilou {

static VkPipelineStageFlags GetVkStageFlagsForLayout(VkImageLayout Layout)
{
	VkPipelineStageFlags Flags = 0;

	switch (Layout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			Flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
			break;
			
		case VK_IMAGE_LAYOUT_GENERAL:
		case VK_IMAGE_LAYOUT_UNDEFINED:
			Flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;

		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		default:
			break;
	}

	return Flags;
}

static VkAccessFlags GetVkAccessMaskForLayout(const VkImageLayout Layout)
{
	VkAccessFlags Flags = 0;

	switch (Layout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			Flags = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			Flags = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			Flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			Flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			Flags = 0;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			Flags = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
			break;

		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			Flags = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
			break;

		case VK_IMAGE_LAYOUT_GENERAL:
			// todo-jn: could be used for R64 in read layout
		case VK_IMAGE_LAYOUT_UNDEFINED:
			Flags = 0;
			break;

		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
			// todo-jn: sync2 currently only used by depth/stencil targets
			Flags = VK_ACCESS_SHADER_READ_BIT;
			break;

		default:
			break;
	}

	return Flags;
}

static void TransitionImageLayout(FVulkanDynamicRHI* Context, RHITexture* Texture, VkImageLayout DstLayout)
{
	VulkanTextureBase* vkTexture = ResourceCast(Texture);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = vkTexture->ImageLayout;
    barrier.newLayout = DstLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vkTexture->Image;
    switch (Texture->GetFormat()) 
    {
    case PF_D32F:
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; break;
    case PF_D24S8:
    case PF_D32FS8:
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; break;
    default:
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; break;
    }
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = Texture->GetNumMips();
    barrier.subresourceRange.layerCount = Texture->GetNumLayers();
    barrier.srcAccessMask = GetVkAccessMaskForLayout(vkTexture->ImageLayout);
    barrier.dstAccessMask = GetVkAccessMaskForLayout(DstLayout);

    VkPipelineStageFlags sourceStage = GetVkStageFlagsForLayout(vkTexture->ImageLayout);
    VkPipelineStageFlags destinationStage = GetVkStageFlagsForLayout(DstLayout);
	FVulkanCmdBuffer* CmdBuffer = Context->CommandBufferManager->GetUploadCmdBuffer();
    vkCmdPipelineBarrier(
        CmdBuffer->GetHandle(),
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    vkTexture->ImageLayout = DstLayout;
}

void FVulkanCommonPipelineDescriptorState::SetUniformBuffer(uint8 BindingIndex, RHIUniformBuffer* Buffer)
{
    VulkanUniformBuffer* vkBuffer = static_cast<VulkanUniformBuffer*>(Buffer);
    FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
    VkDescriptorBufferInfo& Info = Writer.BufferInfo;//std::get<VkDescriptorBufferInfo>(Writer.DescriptorInfo);
    Info.buffer = vkBuffer->GetHandle();
    Info.offset = 0;
    Info.range = vkBuffer->GetSize();
    Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
    Writer.WriteDescriptor.dstBinding = BindingIndex;
    Writer.WriteDescriptor.dstArrayElement = 0;
    Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Writer.WriteDescriptor.descriptorCount = 1;
    Writer.WriteDescriptor.pBufferInfo = &Info;
    vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
}

void FVulkanCommonPipelineDescriptorState::SetBuffer(uint8 BindingIndex, RHIBuffer* Buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(Buffer);
    FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
    VkDescriptorBufferInfo& Info = Writer.BufferInfo;//std::get<VkDescriptorBufferInfo>(Writer.DescriptorInfo);
    Info.buffer = vkBuffer->GetHandle();
    Info.offset = 0;
    Info.range = vkBuffer->GetSize();
    Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
    Writer.WriteDescriptor.dstBinding = BindingIndex;
    Writer.WriteDescriptor.dstArrayElement = 0;
    Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    Writer.WriteDescriptor.descriptorCount = 1;
    Writer.WriteDescriptor.pBufferInfo = &Info;
    vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
}

void FVulkanCommonPipelineDescriptorState::SetSampler(uint8 BindingIndex, FRHISampler Sampler)
{
	TransitionImageLayout(Context, Sampler.Texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanTextureBase* vkTexture = ResourceCast(Sampler.Texture);
    VulkanSamplerState* vkSampler = static_cast<VulkanSamplerState*>(Sampler.SamplerState);
    FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
    VkDescriptorImageInfo& Info = Writer.ImageInfo;//std::get<VkDescriptorImageInfo>(Writer.ImageInfo);
    Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Info.imageView = vkTexture->ImageView;
    Info.sampler = vkSampler->Handle;
    Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
    Writer.WriteDescriptor.dstBinding = BindingIndex;
    Writer.WriteDescriptor.dstArrayElement = 0;
    Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Writer.WriteDescriptor.descriptorCount = 1;
    Writer.WriteDescriptor.pImageInfo = &Info;
    vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
}

void FVulkanCommonPipelineDescriptorState::SetImage(uint8 BindingIndex, RHITexture* Image, EDataAccessFlag Access)
{
	TransitionImageLayout(Context, Image, VK_IMAGE_LAYOUT_GENERAL);
    VulkanTextureBase* vkTexture = ResourceCast(Image);
    FVulkanDescriptorSetWriter& Writer = Writers[BindingIndex];
    VkDescriptorImageInfo& Info = Writer.ImageInfo;//std::get<VkDescriptorImageInfo>(Writer.DescriptorInfo);
	Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    Info.imageView = vkTexture->ImageView;
    Info.sampler = VK_NULL_HANDLE;
    Writer.WriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writer.WriteDescriptor.dstSet = DescriptorSets.Handles[0];
    Writer.WriteDescriptor.dstBinding = BindingIndex;
    Writer.WriteDescriptor.dstArrayElement = 0;
    Writer.WriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ;
    Writer.WriteDescriptor.descriptorCount = 1;
    Writer.WriteDescriptor.pImageInfo = &Info;
    vkUpdateDescriptorSets(Context->device, 1, &Writer.WriteDescriptor, 0, nullptr);
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	for (int i = 0; i < DescriptorSetsLayout->Handles.size(); i++)
		vkDestroyDescriptorSetLayout(Device, DescriptorSetsLayout->Handles[i], nullptr);
}

}