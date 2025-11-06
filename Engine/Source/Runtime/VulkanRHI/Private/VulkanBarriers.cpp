#include "VulkanBarriers.h"
#include "VulkanTexture.h"

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
static void SetupImageBarrier(VkImageMemoryBarrier2& ImgBarrier, VkImage Image, VkPipelineStageFlags SrcStageFlags, VkPipelineStageFlags DstStageFlags, 
	VkAccessFlags SrcAccessFlags, VkAccessFlags DstAccessFlags, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresRange)
{
	ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	ImgBarrier.pNext = nullptr;
	ImgBarrier.srcStageMask = SrcStageFlags;
	ImgBarrier.dstStageMask = DstStageFlags;
	ImgBarrier.srcAccessMask = SrcAccessFlags;
	ImgBarrier.dstAccessMask = DstAccessFlags;
	ImgBarrier.oldLayout = SrcLayout;
	ImgBarrier.newLayout = DstLayout;
	ImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImgBarrier.image = Image;
	ImgBarrier.subresourceRange = SubresRange;
}
// Helper To apply to each bit in an aspect
template <typename TFunction>
static void ForEachAspect(VkImageAspectFlags AspectFlags, TFunction Function)
{
	// Keep it simple for now, can iterate on more bits when needed
	for (uint32 SingleBit = 1; SingleBit <= VK_IMAGE_ASPECT_STENCIL_BIT; SingleBit <<= 1)
	{
		if ((AspectFlags & SingleBit) != 0)
		{
			Function((VkImageAspectFlagBits)SingleBit);
		}
	}
}

bool IsDepthOrStencilAspect(RHITexture* Texture)
{
    return (GetFullAspectMask(Texture->GetFormat()) & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0;
}

static void DowngradeBarrier(VkMemoryBarrier& OutBarrier, const VkMemoryBarrier2& InBarrier)
{
	OutBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	OutBarrier.pNext = InBarrier.pNext;
	OutBarrier.srcAccessMask = InBarrier.srcAccessMask;
	OutBarrier.dstAccessMask = InBarrier.dstAccessMask;
}

static void DowngradeBarrier(VkBufferMemoryBarrier& OutBarrier, const VkBufferMemoryBarrier2& InBarrier)
{
	OutBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	OutBarrier.pNext = InBarrier.pNext;
	OutBarrier.srcAccessMask = InBarrier.srcAccessMask;
	OutBarrier.dstAccessMask = InBarrier.dstAccessMask;
	OutBarrier.srcQueueFamilyIndex = InBarrier.srcQueueFamilyIndex;
	OutBarrier.dstQueueFamilyIndex = InBarrier.dstQueueFamilyIndex;
	OutBarrier.buffer = InBarrier.buffer;
	OutBarrier.offset = InBarrier.offset;
	OutBarrier.size = InBarrier.size;
}
static void DowngradeBarrier(VkImageMemoryBarrier& OutBarrier, const VkImageMemoryBarrier2& InBarrier)
{
	OutBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	OutBarrier.pNext = InBarrier.pNext;
	OutBarrier.srcAccessMask = InBarrier.srcAccessMask;
	OutBarrier.dstAccessMask = InBarrier.dstAccessMask;
	OutBarrier.oldLayout = InBarrier.oldLayout;
	OutBarrier.newLayout = InBarrier.newLayout;
	OutBarrier.srcQueueFamilyIndex = InBarrier.srcQueueFamilyIndex;
	OutBarrier.dstQueueFamilyIndex = InBarrier.dstQueueFamilyIndex;
	OutBarrier.image = InBarrier.image;
	OutBarrier.subresourceRange = InBarrier.subresourceRange;

	// Last minute conversion if both aspects are on the same sync2 layout then we can do the simple conversion (workaround for InitialLayout)
	if (OutBarrier.subresourceRange.aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		if (OutBarrier.newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL)
		{
			OutBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else if (OutBarrier.newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
		{
			OutBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}
}
template <typename DstArrayType, typename SrcArrayType>
static void DowngradeBarrierArray(DstArrayType& TargetArray, const SrcArrayType& SrcArray, VkPipelineStageFlags& MergedSrcStageMask, VkPipelineStageFlags& MergedDstStageMask)
{
	TargetArray.reserve(TargetArray.size() + SrcArray.size());
	for (const auto& SrcBarrier : SrcArray)
	{
		auto& DstBarrier = TargetArray.emplace_back();
		DowngradeBarrier(DstBarrier, SrcBarrier);
		MergedSrcStageMask |= SrcBarrier.srcStageMask;
		MergedDstStageMask |= SrcBarrier.dstStageMask;
	}
}

}