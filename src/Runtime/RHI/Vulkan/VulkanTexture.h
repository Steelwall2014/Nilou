#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "VulkanBarriers.h"

namespace nilou {

class VulkanDevice;

class VulkanTexture : public RHITexture
{
public:
    VkImage Handle{};
    VkDeviceMemory Memory{};
    uint8 BaseMipLevel{};
    uint8 BaseArrayLayer{};
    VkImageAspectFlags FullAspectFlags{};
    VulkanDevice* Device;

    VulkanTexture(
        VkImage InImage,
        VkDeviceMemory InMemory,
        VkImageAspectFlags InFullAspectFlag, 
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

};

// Deprecated typenames
using VulkanTexture2D = VulkanTexture;
using VulkanTexture2DArray = VulkanTexture;
using VulkanTexture3D = VulkanTexture;
using VulkanTextureCube = VulkanTexture;

using VulkanTextureRef = TRefCountPtr<VulkanTexture>;
using VulkanTextureRef = TRefCountPtr<VulkanTexture>;
using VulkanTexture2DRef = TRefCountPtr<VulkanTexture2D>;
using VulkanTexture2DArrayRef = TRefCountPtr<VulkanTexture2DArray>;
using VulkanTexture3DRef = TRefCountPtr<VulkanTexture3D>;
using VulkanTextureCubeRef = TRefCountPtr<VulkanTextureCube>;

inline VulkanTexture* ResourceCast(RHITexture* Texture)
{
    VulkanTexture* vkTextue = static_cast<VulkanTexture*>(Texture);
    return vkTextue;
}

class VulkanTextureView : public RHITextureView
{
public:
    VulkanTextureView(const RHITextureViewDesc& InDesc, RHITexture* InTexture) 
        : RHITextureView(InDesc, InTexture) 
    { }

    VkImageView GetHandle() const { return Handle; }

    VkImageView Handle;
};

inline VulkanTextureView* ResourceCast(RHITextureView* Texture)
{
    VulkanTextureView* vkTextue = static_cast<VulkanTextureView*>(Texture);
    return vkTextue;
}

}