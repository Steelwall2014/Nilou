#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <vector>
#include "VulkanCommandBuffer.h"
#include "VulkanSemaphore.h"
#include "Platform.h"
#include "Common/Log.h"
// #include "VulkanTexture.h"
#include "VulkanResources.h"

// 这个文件里的东西基本上是照抄ue的，因为实在是太烦了

namespace nilou {

struct FVulkanPipelineBarrier
{
	using MemoryBarrierArrayType = std::vector<VkMemoryBarrier2>;
	using ImageBarrierArrayType = std::vector<VkImageMemoryBarrier2>;
	using BufferBarrierArrayType = std::vector<VkBufferMemoryBarrier2>;

	MemoryBarrierArrayType MemoryBarriers;
	ImageBarrierArrayType ImageBarriers;
	BufferBarrierArrayType BufferBarriers;
	std::shared_ptr<FVulkanSemaphore> Semaphore;

	void AddMemoryBarrier(VkAccessFlags SrcAccessFlags, VkAccessFlags DstAccessFlags, VkPipelineStageFlags SrcStageMask, VkPipelineStageFlags DstStageMask);
	void AddImageLayoutTransition(VkImage Image, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresourceRange);
	void AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, const struct FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout);
	void AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, uint32 SrcBaseMip, uint32 SrcBaseLayer, const struct FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout);
	// void AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, VkImageLayout SrcLayout, const struct FVulkanImageLayout& DstLayout);
	// void AddImageLayoutTransition(VkImage Image, VkImageAspectFlags AspectMask, const struct FVulkanImageLayout& SrcLayout, const struct FVulkanImageLayout& DstLayout);
	void AddImageAccessTransition(RHITexture* Surface, ERHIAccess SrcAccess, ERHIAccess DstAccess, const VkImageSubresourceRange& SubresourceRange, VkImageLayout& InOutLayout);
	void Execute(VkCommandBuffer CmdBuffer);
	void Execute(FVulkanCmdBuffer* CmdBuffer);

	static VkImageSubresourceRange MakeSubresourceRange(VkImageAspectFlags AspectMask, uint32 FirstMip = 0, uint32 NumMips = VK_REMAINING_MIP_LEVELS, uint32 FirstLayer = 0, uint32 NumLayers = VK_REMAINING_ARRAY_LAYERS);

};

struct FVulkanImageLayout
{
	FVulkanImageLayout(VkImageLayout InitialLayout, uint32 InNumMips, uint32 InNumLayers/*, VkImageAspectFlags Aspect*/) :
		NumMips(InNumMips),
		NumLayers(InNumLayers),
		/*NumPlanes((Aspect == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) ? 2 : 1),*/
		MainLayout(InitialLayout)
	{
	}

	uint32 NumMips;
	uint32 NumLayers;
	/*uint32 NumPlanes;*/

	// The layout when all the subresources are in the same state.
	VkImageLayout MainLayout;

	// Explicit subresource layouts. Always NumLayers*NumMips elements.
	std::vector<VkImageLayout> SubresLayouts;

	inline bool AreAllSubresourcesSameLayout() const
	{
		return SubresLayouts.size() == 0;
	}

	FVulkanImageLayout GetSubresLayout(const VkImageSubresourceRange& SubresourceRange) const
	{
		FVulkanImageLayout OutLayout{MainLayout, SubresourceRange.levelCount, SubresourceRange.layerCount/*, SubresourceRange.aspectMask*/};
		for (uint32 Mip = 0; Mip < SubresourceRange.levelCount; Mip++)
		{
			for (uint32 Layer = 0; Layer < SubresourceRange.layerCount; Layer++)
			{
				///for (uint32 Plane = 0; Plane < NumPlanes; Plane++)
				//{
					OutLayout.Set(
						GetSubresLayout(Layer+SubresourceRange.baseArrayLayer, Mip+SubresourceRange.baseMipLevel/*, Plane*/), 
						{SubresourceRange.aspectMask, Mip, 1, Layer, 1});
				//}
			}
		}
		OutLayout.CollapseSubresLayoutsIfSame();
		return OutLayout;
	}

	/*VkImageLayout GetSubresLayout(uint32 Layer, uint32 Mip, VkImageAspectFlagBits Aspect) const
	{
		return GetSubresLayout(Layer, Mip, (Aspect==VK_IMAGE_ASPECT_STENCIL_BIT) ? NumPlanes - 1 : 0);
	}*/

	VkImageLayout GetSubresLayout(uint32 Layer, uint32 Mip/*, uint32 Plane*/) const
	{
		if (SubresLayouts.size() == 0)
		{
			return MainLayout;
		}

		if (Layer == (uint32)-1)
		{
			Layer = 0;
		}

		Ncheck(/*Plane < NumPlanes && */Layer < NumLayers && Mip < NumMips);
		return SubresLayouts[/*(Plane * NumLayers * NumMips) + */(Layer * NumMips) + Mip];
	}

	bool AreSubresourcesSameLayout(VkImageLayout Layout, const VkImageSubresourceRange& SubresourceRange) const
    {
        if (SubresLayouts.size() == 0)
        {
            return MainLayout == Layout;
        }

        /*const uint32 FirstPlane = (SubresourceRange.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) ? NumPlanes - 1 : 0;
        const uint32 LastPlane = (SubresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) ? NumPlanes : 1;*/

        const uint32 FirstLayer = SubresourceRange.baseArrayLayer;
        const uint32 LastLayer = FirstLayer + GetSubresRangeLayerCount(SubresourceRange);

        const uint32 FirstMip = SubresourceRange.baseMipLevel;
        const uint32 LastMip = FirstMip + GetSubresRangeMipCount(SubresourceRange);

        //for (uint32 PlaneIdx = FirstPlane; PlaneIdx < LastPlane; ++PlaneIdx)
        //{
            for (uint32 LayerIdx = FirstLayer; LayerIdx < LastLayer; ++LayerIdx)
            {
                for (uint32 MipIdx = FirstMip; MipIdx < LastMip; ++MipIdx)
                {
                    if (SubresLayouts[/*(PlaneIdx * NumLayers * NumMips) + */(LayerIdx * NumMips) + MipIdx] != Layout)
                    {
                        return false;
                    }
                }
            }
        //}

        return true;
    }

	inline uint32 GetSubresRangeLayerCount(const VkImageSubresourceRange& SubresourceRange) const
	{
		Ncheck(SubresourceRange.baseArrayLayer < NumLayers);
		if (SubresourceRange.baseArrayLayer >= NumLayers)
			throw "";
		return (SubresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (NumLayers - SubresourceRange.baseArrayLayer) : SubresourceRange.layerCount;
	}

	inline uint32 GetSubresRangeMipCount(const VkImageSubresourceRange& SubresourceRange) const
	{
		Ncheck(SubresourceRange.baseMipLevel < NumMips);
		return (SubresourceRange.levelCount == VK_REMAINING_MIP_LEVELS) ? (NumMips - SubresourceRange.baseMipLevel) : SubresourceRange.levelCount;
	}

	void CollapseSubresLayoutsIfSame()
    {
        if (SubresLayouts.size() == 0)
        {
            return;
        }

        const VkImageLayout Layout = SubresLayouts[0];
        for (uint32 i = 1; i < /*NumPlanes * */NumLayers * NumMips; ++i)
        {
            if (SubresLayouts[i] != Layout)
            {
                return;
            }
        }

        MainLayout = Layout;
        SubresLayouts.clear();
    }

	void Set(VkImageLayout Layout, const VkImageSubresourceRange& SubresourceRange)
    {
        //const uint32 FirstPlane = (SubresourceRange.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) ? NumPlanes - 1 : 0;
        //const uint32 LastPlane = (SubresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) ? NumPlanes : 1;

        const uint32 FirstLayer = SubresourceRange.baseArrayLayer;
        const uint32 LayerCount = GetSubresRangeLayerCount(SubresourceRange);

        const uint32 FirstMip = SubresourceRange.baseMipLevel;
        const uint32 MipCount = GetSubresRangeMipCount(SubresourceRange);

        if (/*FirstPlane == 0 && LastPlane == NumPlanes &&*/
            FirstLayer == 0 && LayerCount == NumLayers && 
            FirstMip == 0 && MipCount == NumMips)
        {
            // We're setting the entire resource to the same layout.
            MainLayout = Layout;
            SubresLayouts.clear();
            return;
        }

        if (SubresLayouts.size() == 0)
        {
            const uint32 SubresLayoutCount = /*NumPlanes * */NumLayers * NumMips;
            SubresLayouts.resize(SubresLayoutCount);
            for (uint32 i = 0; i < SubresLayoutCount; ++i)
            {
                SubresLayouts[i] = MainLayout;
            }
        }

        //for (uint32 Plane = FirstPlane; Plane < LastPlane; ++Plane)
        //{
            for (uint32 Layer = FirstLayer; Layer < FirstLayer + LayerCount; ++Layer)
            {
                for (uint32 Mip = FirstMip; Mip < FirstMip + MipCount; ++Mip)
                {
                    SubresLayouts[/*Plane * (NumLayers * NumMips)*/ + Layer * NumMips + Mip] = Layout;
                }
            }
        //}

        // It's possible we've just set all the subresources to the same layout. If that's the case, get rid of the
        // subresource info and set the main layout appropriatedly.
        CollapseSubresLayoutsIfSame();
    }
};

class FVulkanLayoutManager
{
public:
	FVulkanLayoutManager(bool InWriteOnly, FVulkanLayoutManager* InFallback)
		: bWriteOnly(InWriteOnly)
		, Fallback(InFallback)
	{
	}

	void NotifyDeletedImage(VkImage Image);

	// Predetermined layouts for given RHIAccess
	static VkImageLayout GetDefaultLayout(FVulkanCmdBuffer* CmdBuffer, RHITexture* Texture, ERHIAccess DesiredAccess);

	// Expected layouts and Hints are workarounds until we can use 'hardcoded layouts' everywhere.
	static VkImageLayout SetExpectedLayout(FVulkanCmdBuffer* CmdBuffer, RHITexture* Texture, ERHIAccess DesiredAccess);
	VkImageLayout GetDepthStencilHint(RHITexture* Texture, VkImageAspectFlagBits AspectBit);

	const FVulkanImageLayout* GetFullLayout(VkImage Image) const
	{
		Ncheck(!bWriteOnly);
		const FVulkanImageLayout* Layout = Find(Image);
		if (!Layout && Fallback)
		{
			return Fallback->GetFullLayout(Image);
		}
		return Layout;
	}

	const FVulkanImageLayout* GetFullLayout(RHITexture* Texture, bool bAddIfNotFound = false, VkImageLayout LayoutIfNotFound = VK_IMAGE_LAYOUT_UNDEFINED);

	// Not the preferred path because we can't ensure Mip and Layer counts match, but still necessary for images like the backbuffer
	void SetFullLayout(VkImage Image, const FVulkanImageLayout& NewLayout)
	{
		FVulkanImageLayout* Layout = Find(Image);
		if (Layout)
		{
			*Layout = NewLayout;
		}
		else
		{
			Layouts.insert({Image, NewLayout});
		}
	}

	void SetFullLayout(RHITexture* Texture, const FVulkanImageLayout& NewLayout);

	void SetFullLayout(RHITexture* Texture, VkImageLayout InLayout, bool bOnlyIfNotFound=false);

	void SetLayout(RHITexture* Texture, const VkImageSubresourceRange& InSubresourceRange, VkImageLayout InLayout);

	void UpdateLayout(VkImage Image, const VkImageSubresourceRange& InSubresourceRange, VkImageLayout InLayout)
	{
		FVulkanImageLayout* Layout = Find(Image);
		if (Layout)
		{
			Layout->Set(InLayout, InSubresourceRange);
		}
	}

private:

    const FVulkanImageLayout* Find(VkImage Image) const
    {
        auto Found = Layouts.find(Image);
		const FVulkanImageLayout* Layout = Found == Layouts.end() ? nullptr : &Found->second;
        return Layout;
    }

    FVulkanImageLayout* Find(VkImage Image)
    {
        auto Found = Layouts.find(Image);
		FVulkanImageLayout* Layout = Found == Layouts.end() ? nullptr : &Found->second;
        return Layout;
    }

	std::unordered_map<VkImage, FVulkanImageLayout> Layouts;

	// If we're WriteOnly, we should never read layout from this instance.  This is important for parallel rendering.
	// When WriteOnly, this instance of the layout manager should only collect layouts to later feed them to the another central mgr.
	const bool bWriteOnly;

	// If parallel command list creation is NOT supported, then the queue's layout mgr can be used as a fallback to fetch previous layouts.
	FVulkanLayoutManager* Fallback;
};
inline FVulkanLayoutManager& GetLayoutManager()
{
    static FVulkanLayoutManager Mng(false, nullptr);
    return Mng;
}
class VulkanTextureBase;
struct FVulkanImageLayoutBarrierHelper
{
	FVulkanPipelineBarrier Barrier;
	
	void AddImageLayoutTransition(VulkanTextureBase* Image, VkImageLayout SrcLayout, VkImageLayout DstLayout, const VkImageSubresourceRange& SubresourceRange);
	void AddImageLayoutTransition(VulkanTextureBase* Image, VkImageAspectFlags AspectMask, const struct FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout);
	// void AddImageLayoutTransition(VulkanTextureBase* Image, VkImageAspectFlags AspectMask, uint32 SrcBaseMip, uint32 SrcNumMips, uint32 SrcBaseLayer, uint32 SrcNumLayers, const struct FVulkanImageLayout& SrcLayout, VkImageLayout DstLayout);
	// void AddImageLayoutTransition(VulkanTextureBase* Image, VkImageAspectFlags AspectMask, VkImageLayout SrcLayout, const struct FVulkanImageLayout& DstLayout);
	// void AddImageLayoutTransition(VulkanTextureBase* Image, VkImageAspectFlags AspectMask, const struct FVulkanImageLayout& SrcLayout, const struct FVulkanImageLayout& DstLayout);
	void Execute(VkCommandBuffer CmdBuffer) { Barrier.Execute(CmdBuffer); }
	void Execute(FVulkanCmdBuffer* CmdBuffer)  { Barrier.Execute(CmdBuffer); }
};

}
