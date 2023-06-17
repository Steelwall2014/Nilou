#include "VulkanTexture.h"
#include "VulkanDynamicRHI.h"

namespace nilou {

VulkanTextureBase::~VulkanTextureBase()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    vkDestroyImage(RHI->device, Image, nullptr);
    vkFreeMemory(RHI->device, Memory, nullptr);
}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTexture2D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY)
{
    VulkanTexture2DRef Texture = std::static_pointer_cast<VulkanTexture2D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 1, ETextureType::TT_Texture2D));
    return Texture;
}

RHITexture2DArrayRef FVulkanDynamicRHI::RHICreateTexture2DArray(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ)
{
    VulkanTexture2DArrayRef Texture = std::static_pointer_cast<VulkanTexture2DArray>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 1, ETextureType::TT_Texture2DArray));
    return Texture;
}

RHITexture3DRef FVulkanDynamicRHI::RHICreateTexture3D(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ)
{
    VulkanTexture3DRef Texture = std::static_pointer_cast<VulkanTexture3D>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, InSizeY, ETextureType::TT_Texture3D));
    return Texture;
}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureCube(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY)
{
    VulkanTextureCubeRef Texture = std::static_pointer_cast<VulkanTextureCube>(
        RHICreateTextureInternal(
            name, Format, NumMips, 
            InSizeX, InSizeY, 6, ETextureType::TT_TextureCube));
    return Texture;
}

RHITextureRef FVulkanDynamicRHI::RHICreateTextureInternal(
    const std::string &name, EPixelFormat Format, 
    int32 NumMips, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, ETextureType TextureType)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.extent.width = InSizeX;
    imageInfo.extent.height = InSizeY;
    imageInfo.mipLevels = NumMips;
    if (TextureType == ETextureType::TT_Texture2D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = 1;
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = InSizeZ;
        imageInfo.extent.depth = 1;
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = InSizeZ;
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 6;
        imageInfo.extent.depth = 1;
    }
    imageInfo.format = TranslatePixelFormatToVKFormat(Format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage Image{};
    VkDeviceMemory Memory{};

    if (vkCreateImage(device, &imageInfo, nullptr, &Image) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create image!")
        return nullptr;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &Memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, Image, Memory, 0);
    
    if (TextureType == ETextureType::TT_Texture2D)
    {
        return std::make_shared<VulkanTexture2D>(Image, Memory, InSizeX, InSizeY, 1, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        return std::make_shared<VulkanTexture2DArray>(Image, Memory, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        return std::make_shared<VulkanTexture3D>(Image, Memory, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        return std::make_shared<VulkanTextureCube>(Image, Memory, InSizeX, InSizeY, 6, NumMips, Format, name);
    }
    return nullptr;

}

}