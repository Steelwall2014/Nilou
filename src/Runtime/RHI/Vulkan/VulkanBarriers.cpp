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

void FVulkanPipelineBarrier::AddMemoryBarrier(VkAccessFlags InSrcAccessFlags, VkAccessFlags InDstAccessFlags, VkPipelineStageFlags InSrcStageMask, VkPipelineStageFlags InDstStageMask)
{
	const VkAccessFlags ReadMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
		VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

	if (MemoryBarriers.size() == 0)
	{
		MemoryBarriers.emplace_back();
	}

	// Mash everything into a single barrier
	VkMemoryBarrier2& MemoryBarrier = MemoryBarriers[0];

	// We only need a memory barrier if the previous commands wrote to the buffer. In case of a transition from read, an execution barrier is enough.
	const bool SrcAccessIsRead = ((InSrcAccessFlags & (~ReadMask)) == 0);
	if (!SrcAccessIsRead)
	{
		MemoryBarrier.srcAccessMask |= InSrcAccessFlags;
		MemoryBarrier.dstAccessMask |= InDstAccessFlags;
	}

	MemoryBarrier.srcStageMask |= InSrcStageMask;
	MemoryBarrier.dstStageMask |= InDstStageMask;
}

void FVulkanPipelineBarrier::AddImageLayoutTransition(VkImage Image, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresourceRange)
{
	const VkPipelineStageFlags SrcStageMask = GetVkStageFlagsForLayout(SrcLayout);
	const VkPipelineStageFlags DstStageMask = GetVkStageFlagsForLayout(DstLayout);

	const VkAccessFlags SrcAccessFlags = GetVkAccessMaskForLayout(SrcLayout);
	const VkAccessFlags DstAccessFlags = GetVkAccessMaskForLayout(DstLayout);

	VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
	SetupImageBarrier(ImgBarrier, Image, SrcStageMask, DstStageMask, SrcAccessFlags, DstAccessFlags, SrcLayout, DstLayout, SubresourceRange);
}



void FVulkanPipelineBarrier::AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, const FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout)
{
	if (SrcLayout.AreAllSubresourcesSameLayout())
	{
		AddImageLayoutTransition(Image, SrcLayout.MainLayout, DstLayout, MakeSubresourceRange(AspectMask));
		return;
	}

	const VkPipelineStageFlags DstStageMask = GetVkStageFlagsForLayout(DstLayout);
	const VkAccessFlags DstAccessFlags = GetVkAccessMaskForLayout(DstLayout);

	ForEachAspect(AspectMask, [&](VkImageAspectFlagBits SingleAspect)
		{
			VkImageSubresourceRange SubresourceRange = MakeSubresourceRange(AspectMask, 0, 1, 0, 1);
			for (; SubresourceRange.baseArrayLayer < SrcLayout.NumLayers; ++SubresourceRange.baseArrayLayer)
			{
				for (SubresourceRange.baseMipLevel = 0; SubresourceRange.baseMipLevel < SrcLayout.NumMips; ++SubresourceRange.baseMipLevel)
				{
					const VkImageLayout SubresourceLayout = SrcLayout.GetSubresLayout(SubresourceRange.baseArrayLayer, SubresourceRange.baseMipLevel, SingleAspect);
					if (SubresourceLayout != DstLayout)
					{
						const VkPipelineStageFlags SrcStageMask = GetVkStageFlagsForLayout(SubresourceLayout);
						const VkAccessFlags SrcAccessFlags = GetVkAccessMaskForLayout(SubresourceLayout);

						VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
						SetupImageBarrier(ImgBarrier, Image, SrcStageMask, DstStageMask, SrcAccessFlags, DstAccessFlags, SubresourceLayout, DstLayout, SubresourceRange);
					}
				}
			}
		});
}

void FVulkanPipelineBarrier::AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, uint32 SrcBaseMip, uint32 SrcNumMips, uint32 SrcBaseLayer, uint32 SrcNumLayers, const FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout)
{
	if (SrcLayout.AreAllSubresourcesSameLayout())
	{
		AddImageLayoutTransition(Image, SrcLayout.MainLayout, DstLayout, MakeSubresourceRange(AspectMask));
		return;
	}

	const VkPipelineStageFlags DstStageMask = GetVkStageFlagsForLayout(DstLayout);
	const VkAccessFlags DstAccessFlags = GetVkAccessMaskForLayout(DstLayout);

	ForEachAspect(AspectMask, [&](VkImageAspectFlagBits SingleAspect)
		{
			VkImageSubresourceRange SubresourceRange = MakeSubresourceRange(AspectMask, SrcBaseMip, 1, SrcBaseLayer, 1);
			for (; SubresourceRange.baseArrayLayer < SrcBaseLayer+SrcNumLayers; ++SubresourceRange.baseArrayLayer)
			{
				for (SubresourceRange.baseMipLevel = SrcBaseMip; SubresourceRange.baseMipLevel < SrcBaseMip+SrcNumMips; ++SubresourceRange.baseMipLevel)
				{
					const VkImageLayout SubresourceLayout = SrcLayout.GetSubresLayout(SubresourceRange.baseArrayLayer, SubresourceRange.baseMipLevel, SingleAspect);
					if (SubresourceLayout != DstLayout)
					{
						const VkPipelineStageFlags SrcStageMask = GetVkStageFlagsForLayout(SubresourceLayout);
						const VkAccessFlags SrcAccessFlags = GetVkAccessMaskForLayout(SubresourceLayout);

						VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
						SetupImageBarrier(ImgBarrier, Image, SrcStageMask, DstStageMask, SrcAccessFlags, DstAccessFlags, SubresourceLayout, DstLayout, SubresourceRange);
					}
				}
			}
		});
}

void FVulkanPipelineBarrier::AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, VkImageLayout SrcLayout, const FVulkanImageLayout& DstLayout)
{
	if (DstLayout.AreAllSubresourcesSameLayout())
	{
		AddImageLayoutTransition(Image, SrcLayout, DstLayout.MainLayout, MakeSubresourceRange(AspectMask));
		return;
	}

	const VkPipelineStageFlags SrcStageMask = GetVkStageFlagsForLayout(SrcLayout);
	const VkAccessFlags SrcAccessFlags = GetVkAccessMaskForLayout(SrcLayout);

	ForEachAspect(AspectMask, [&](VkImageAspectFlagBits SingleAspect)
		{
			VkImageSubresourceRange SubresourceRange = MakeSubresourceRange(AspectMask, 0, 1, 0, 1);
			for (; SubresourceRange.baseArrayLayer < DstLayout.NumLayers; ++SubresourceRange.baseArrayLayer)
			{
				for (SubresourceRange.baseMipLevel = 0; SubresourceRange.baseMipLevel < DstLayout.NumMips; ++SubresourceRange.baseMipLevel)
				{
					const VkImageLayout SubresourceLayout = DstLayout.GetSubresLayout(SubresourceRange.baseArrayLayer, SubresourceRange.baseMipLevel, SingleAspect);
					if (SubresourceLayout != SrcLayout)
					{
						const VkPipelineStageFlags DstStageMask = GetVkStageFlagsForLayout(SubresourceLayout);
						const VkAccessFlags DstAccessFlags = GetVkAccessMaskForLayout(SubresourceLayout);

						VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
						SetupImageBarrier(ImgBarrier, Image, SrcStageMask, DstStageMask, SrcAccessFlags, DstAccessFlags, SrcLayout, SubresourceLayout, SubresourceRange);
					}
				}
			}
		});
}

void FVulkanPipelineBarrier::AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, const FVulkanImageLayout& SrcLayout, const FVulkanImageLayout& DstLayout)
{
	if (SrcLayout.AreAllSubresourcesSameLayout())
	{
		AddImageLayoutTransition(Image, AspectMask, SrcLayout.MainLayout, DstLayout);
	}
	else if (DstLayout.AreAllSubresourcesSameLayout())
	{
		AddImageLayoutTransition(Image, AspectMask, SrcLayout, DstLayout.MainLayout);
	}
	else
	{
		Ncheck(SrcLayout.NumLayers == DstLayout.NumLayers);
		Ncheck(SrcLayout.NumMips == DstLayout.NumMips);

		ForEachAspect(AspectMask, [&](VkImageAspectFlagBits SingleAspect)
			{
				VkImageSubresourceRange SubresourceRange = MakeSubresourceRange(AspectMask, 0, 1, 0, 1);
				for (; SubresourceRange.baseArrayLayer < DstLayout.NumLayers; ++SubresourceRange.baseArrayLayer)
				{
					for (SubresourceRange.baseMipLevel = 0; SubresourceRange.baseMipLevel < DstLayout.NumMips; ++SubresourceRange.baseMipLevel)
					{
						const VkImageLayout SrcSubresourceLayout = SrcLayout.GetSubresLayout(SubresourceRange.baseArrayLayer, SubresourceRange.baseMipLevel, SingleAspect);
						const VkImageLayout DstSubresourceLayout = DstLayout.GetSubresLayout(SubresourceRange.baseArrayLayer, SubresourceRange.baseMipLevel, SingleAspect);
						if (SrcSubresourceLayout != DstSubresourceLayout)
						{
							const VkPipelineStageFlags SrcStageMask = GetVkStageFlagsForLayout(SrcSubresourceLayout);
							const VkAccessFlags SrcAccessFlags = GetVkAccessMaskForLayout(SrcSubresourceLayout);

							const VkPipelineStageFlags DstStageMask = GetVkStageFlagsForLayout(DstSubresourceLayout);
							const VkAccessFlags DstAccessFlags = GetVkAccessMaskForLayout(DstSubresourceLayout);

							VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
							SetupImageBarrier(ImgBarrier, Image, SrcStageMask, DstStageMask, SrcAccessFlags, DstAccessFlags, SrcSubresourceLayout, DstSubresourceLayout, SubresourceRange);
						}
					}
				}
			});
	}
}

bool IsDepthOrStencilAspect(RHITexture* Texture)
{
    return (GetFullAspectMask(Texture->GetFormat()) & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0;
}

// void FVulkanPipelineBarrier::AddImageAccessTransition(RHITexture* Surface, ERHIAccess SrcAccess, ERHIAccess DstAccess, const VkImageSubresourceRange& SubresourceRange, VkImageLayout& InOutLayout)
// {
//     VulkanTextureBase* VulkanTexture = ResourceCast(Surface);
// 	// This function should only be used for known states.
// 	Ncheck(DstAccess != ERHIAccess::Unknown);
// 	const bool bIsDepthStencil = IsDepthOrStencilAspect(Surface);
// 	const bool bSupportsReadOnlyOptimal = Surface.SupportsSampling();

// 	VkPipelineStageFlags ImgSrcStage, ImgDstStage;
// 	VkAccessFlags SrcAccessFlags, DstAccessFlags;
// 	VkImageLayout SrcLayout, DstLayout;
// 	GetVkStageAndAccessFlags(SrcAccess, FRHITransitionInfo::EType::Texture, 0, bIsDepthStencil, bSupportsReadOnlyOptimal, ImgSrcStage, SrcAccessFlags, SrcLayout, true);
// 	GetVkStageAndAccessFlags(DstAccess, FRHITransitionInfo::EType::Texture, 0, bIsDepthStencil, bSupportsReadOnlyOptimal, ImgDstStage, DstAccessFlags, DstLayout, false);

// 	// If not compute, remove vertex pipeline bits as only compute updates vertex buffers
// 	if (!(ImgSrcStage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT))
// 	{
// 		ImgDstStage &= ~(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
// 	}

// 	if (SrcLayout == VK_IMAGE_LAYOUT_UNDEFINED)
// 	{
// 		SrcLayout = InOutLayout;
// 		SrcAccessFlags = GetVkAccessMaskForLayout(SrcLayout);
// 	}
// 	else
// 	{
// 		assert(SrcLayout == InOutLayout);
// 	}

// 	if (DstLayout == VK_IMAGE_LAYOUT_UNDEFINED)
// 	{
// 		DstLayout = VK_IMAGE_LAYOUT_GENERAL;
// 	}

// 	VkImageMemoryBarrier2& ImgBarrier = ImageBarriers.emplace_back();
// 	SetupImageBarrier(ImgBarrier, VulkanTexture->Image, ImgSrcStage, ImgDstStage, SrcAccessFlags, DstAccessFlags, SrcLayout, DstLayout, SubresourceRange);

// 	InOutLayout = DstLayout;
// }


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
void FVulkanPipelineBarrier::Execute(VkCommandBuffer CmdBuffer)
{
	if (MemoryBarriers.size() != 0 || BufferBarriers.size() != 0 || ImageBarriers.size() != 0)
	{
		VkPipelineStageFlags SrcStageMask = 0;
		VkPipelineStageFlags DstStageMask = 0;

		std::vector<VkMemoryBarrier> TempMemoryBarriers;
		DowngradeBarrierArray(TempMemoryBarriers, MemoryBarriers, SrcStageMask, DstStageMask);

		std::vector<VkBufferMemoryBarrier> TempBufferBarriers;
		DowngradeBarrierArray(TempBufferBarriers, BufferBarriers, SrcStageMask, DstStageMask);

		std::vector<VkImageMemoryBarrier> TempImageBarriers;
		DowngradeBarrierArray(TempImageBarriers, ImageBarriers, SrcStageMask, DstStageMask);

		vkCmdPipelineBarrier(CmdBuffer, SrcStageMask, DstStageMask, 0, TempMemoryBarriers.size(), TempMemoryBarriers.data(), 
			TempBufferBarriers.size(), TempBufferBarriers.data(), TempImageBarriers.size(), TempImageBarriers.data());
	}
}

void FVulkanPipelineBarrier::Execute(FVulkanCmdBuffer* CmdBuffer)
{
	if (MemoryBarriers.size() != 0 || BufferBarriers.size() != 0 || ImageBarriers.size() != 0)
	{
        Execute(CmdBuffer->GetHandle());
	}
}

VkImageSubresourceRange FVulkanPipelineBarrier::MakeSubresourceRange(VkImageAspectFlags AspectMask, uint32 FirstMip, uint32 NumMips, uint32 FirstLayer, uint32 NumLayers)
{
	VkImageSubresourceRange Range;
	Range.aspectMask = AspectMask;
	Range.baseMipLevel = FirstMip;
	Range.levelCount = NumMips;
	Range.baseArrayLayer = FirstLayer;
	Range.layerCount = NumLayers;
	return Range;
}

const FVulkanImageLayout* FVulkanLayoutManager::GetFullLayout(RHITexture* Texture, bool bAddIfNotFound, VkImageLayout LayoutIfNotFound)
{
    VulkanTextureBase* VulkanTexture = ResourceCast(Texture);
    Ncheck(!bWriteOnly);
    const FVulkanImageLayout* Layout = Find(VulkanTexture->ImageView);
    
    if (!Layout && Fallback)
    {
        Layout = Fallback->GetFullLayout(Texture, false);
    }
    
    if (Layout)
    {
        return Layout;
    }
    else if (!bAddIfNotFound)
    {
        return nullptr;
    }

    auto insert = Layouts.insert({VulkanTexture->ImageView, FVulkanImageLayout(LayoutIfNotFound, Texture->GetNumMips(), Texture->GetNumLayers(), GetFullAspectMask(Texture->GetFormat()))});
    return &insert.first->second;
}

void FVulkanLayoutManager::SetFullLayout(RHITexture* Texture, const FVulkanImageLayout& NewLayout)
{
    Ncheck((Texture->GetNumMips() == NewLayout.NumMips) && (Texture->GetNumLayers() == NewLayout.NumLayers));
    VulkanTextureBase* VulkanTexture = ResourceCast(Texture);
    SetFullLayout(VulkanTexture->ImageView, NewLayout);
}

void FVulkanLayoutManager::SetFullLayout(RHITexture* Texture, VkImageLayout InLayout, bool bOnlyIfNotFound)
{
    VulkanTextureBase* VulkanTexture = ResourceCast(Texture);
    FVulkanImageLayout* Layout = Find(VulkanTexture->ImageView);
    if (Layout)
    {
        if (!bOnlyIfNotFound)
        {
            Layout->Set(InLayout, FVulkanPipelineBarrier::MakeSubresourceRange(GetFullAspectMask(Texture->GetFormat())));
        }
    }
    else
    {
        Layouts.insert({VulkanTexture->ImageView, FVulkanImageLayout(InLayout, Texture->GetNumMips(), Texture->GetNumLayers(), GetFullAspectMask(Texture->GetFormat()))});
    }
}

void FVulkanLayoutManager::SetLayout(RHITexture* Texture, const VkImageSubresourceRange& InSubresourceRange, VkImageLayout InLayout)
{
    VulkanTextureBase* VulkanTexture = ResourceCast(Texture);
    FVulkanImageLayout* Layout = Find(VulkanTexture->ImageView);
    if (Layout)
    {
        Layout->Set(InLayout, InSubresourceRange);
    }
    else
    {
        FVulkanImageLayout NewLayout(VK_IMAGE_LAYOUT_UNDEFINED, Texture->GetNumMips(), Texture->GetNumLayers(), GetFullAspectMask(Texture->GetFormat()));
        NewLayout.Set(InLayout, InSubresourceRange);
        Layouts.insert({VulkanTexture->ImageView, NewLayout});
    }
}

}