#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "VulkanBarriers.h"

namespace nilou {


class VulkanTexture : public RHITexture
{
public:
    VkImage Handle{};
    VkImageView ImageView{};
    VkDeviceMemory Memory{};
    uint8 BaseMipLevel{};
    uint8 BaseArrayLayer{};
    VulkanTexture* ParentTexture{};
    VkImageAspectFlags FullAspectFlags{};

    #ifdef NILOU_DEBUG
    FVulkanImageLayout DebugLayout;
    #endif

    VulkanTexture(
        VulkanTexture* InParentTexture,
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        VkImageAspectFlags InFullAspectFlag, 
        const FVulkanImageLayout& InImageLayout,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InNumMips,
        uint32 InBaseMipLevel,
        uint32 InBaseArrayLayer,
        EPixelFormat InFormat,
        const std::string &InTextureName,
        ETextureDimension InTextureType
    );
    ~VulkanTexture();
    FVulkanImageLayout GetImageLayout() const;
    VkImageView GetImageView() const { return ImageView; }
    void SetImageLayout(VkImageLayout Layout, const VkImageSubresourceRange& Range);
    void SetFullImageLayout(VkImageLayout Layout);
    bool IsImageView() const { return ParentTexture != nullptr; }

};

// Deprecated typenames
using VulkanTexture = VulkanTexture;
using VulkanTexture2D = VulkanTexture;
using VulkanTexture2DArray = VulkanTexture;
using VulkanTexture3D = VulkanTexture;
using VulkanTextureCube = VulkanTexture;

using VulkanTextureRef = std::shared_ptr<VulkanTexture>;
using VulkanTextureRef = std::shared_ptr<VulkanTexture>;
using VulkanTexture2DRef = std::shared_ptr<VulkanTexture2D>;
using VulkanTexture2DArrayRef = std::shared_ptr<VulkanTexture2DArray>;
using VulkanTexture3DRef = std::shared_ptr<VulkanTexture3D>;
using VulkanTextureCubeRef = std::shared_ptr<VulkanTextureCube>;

inline VulkanTexture* ResourceCast(RHITexture* Texture)
{
    VulkanTexture* vkTextue = static_cast<VulkanTexture*>(Texture);
    return vkTextue;
}

}