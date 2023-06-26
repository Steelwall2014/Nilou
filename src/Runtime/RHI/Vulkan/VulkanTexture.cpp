#include "VulkanTexture.h"
#include "VulkanDynamicRHI.h"
#include "VulkanMemory.h"

namespace nilou {

VulkanTextureBase::~VulkanTextureBase()
{
    FVulkanDynamicRHI* RHI = static_cast<FVulkanDynamicRHI*>(FDynamicRHI::GetDynamicRHI());
    if (Image)
        vkDestroyImage(RHI->device, Image, nullptr);
    if (Memory)
        RHI->MemoryManager->FreeMemory(Memory);
    if (ImageView)
        vkDestroyImageView(RHI->device, ImageView, nullptr);
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
    VkImageViewCreateInfo viewInfo{};
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
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = InSizeZ;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.arrayLayers = 1;
        imageInfo.extent.depth = InSizeZ;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.arrayLayers = 6;
        imageInfo.extent.depth = 1;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    imageInfo.format = TranslatePixelFormatToVKFormat(Format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage Image{};
    VkImageView ImageView{};
    VkDeviceMemory Memory{};

    if (vkCreateImage(device, &imageInfo, nullptr, &Image) != VK_SUCCESS) {
        NILOU_LOG(Error, "failed to create image!")
        return nullptr;
    }

    MemoryManager->AllocateImageMemory(&Memory, Image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindImageMemory(device, Image, Memory, 0);

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = NumMips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    if (TextureType == ETextureType::TT_Texture2D)
    {
        return std::make_shared<VulkanTexture2D>(Image, ImageView, Memory, InSizeX, InSizeY, 1, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture2DArray)
    {
        return std::make_shared<VulkanTexture2DArray>(Image, ImageView, Memory, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_Texture3D)
    {
        return std::make_shared<VulkanTexture3D>(Image, ImageView, Memory, InSizeX, InSizeY, InSizeZ, NumMips, Format, name);
    }
    else if (TextureType == ETextureType::TT_TextureCube)
    {
        return std::make_shared<VulkanTextureCube>(Image, ImageView, Memory, InSizeX, InSizeY, 6, NumMips, Format, name);
    }
    return nullptr;

}

RHITexture2DRef FVulkanDynamicRHI::RHICreateTextureView2D(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels, uint32 LevelIndex)
{
    VulkanTexture* Texture = static_cast<VulkanTexture*>(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->GetImage();
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = LevelIndex;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    return std::make_shared<VulkanTexture2D>(
        VK_NULL_HANDLE, ImageView, VK_NULL_HANDLE, 
        Texture->GetSizeXYZ().x, Texture->GetSizeXYZ().y, 1, 
        NumLevels, Format, OriginTexture->GetName()+"_View");

}

RHITextureCubeRef FVulkanDynamicRHI::RHICreateTextureViewCube(RHITexture* OriginTexture, EPixelFormat Format, uint32 MinLevel, uint32 NumLevels)
{
    VulkanTexture* Texture = static_cast<VulkanTexture*>(OriginTexture);
    VkImageViewCreateInfo viewInfo{};
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    switch (Format) 
    {
    case EPixelFormat::PF_D32F:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case EPixelFormat::PF_D24S8:
    case EPixelFormat::PF_D32FS8:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageView ImageView{};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Texture->GetImage();
    viewInfo.format = TranslatePixelFormatToVKFormat(Format);
    viewInfo.subresourceRange.baseMipLevel = MinLevel;
    viewInfo.subresourceRange.levelCount = NumLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

    vkCreateImageView(device, &viewInfo, nullptr, &ImageView);

    return std::make_shared<VulkanTextureCube>(
        VK_NULL_HANDLE, ImageView, VK_NULL_HANDLE, 
        Texture->GetSizeXYZ().x, Texture->GetSizeXYZ().y, 6, 
        NumLevels, Format, OriginTexture->GetName()+"_View");

}

}