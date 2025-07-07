#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "VulkanBarriers.h"

namespace nilou {

class VulkanDevice;

class VulkanTexture : public RHITexture
{
public:
    VkImage Handle = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkImageAspectFlags FullAspectFlags = VK_IMAGE_ASPECT_NONE;
    VulkanDevice* Device = nullptr;

    VulkanTexture(
        VulkanDevice* InDevice,
        VkImage InImage,
        VkDeviceMemory InMemory,
        VkImageAspectFlags InFullAspectFlag, 
        const std::string &InTextureName,
        RHITextureDesc InDesc
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