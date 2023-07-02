#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {


class VulkanTextureBase
{
public:
    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    VkImageLayout ImageLayout;

    VulkanTextureBase(
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        VkImageLayout InImageLayout
    )
    : Image(InImage)
    , ImageView(InImageView)
    , Memory(InMemory)
    , ImageLayout(InImageLayout)
    { }
    ~VulkanTextureBase();

};

template<typename BaseType>
class TVulkanTexture : public BaseType
{
public:
    TVulkanTexture(
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        VkImageLayout InImageLayout,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InNumMips,
        EPixelFormat InFormat,
        const std::string &InTextureName
    )
    : TextureBase(InImage, InImageView, InMemory, InImageLayout)
    , BaseType(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    {}
    VkImage GetImage() const { return TextureBase.Image; }
    VkImageView GetImageView() const { return TextureBase.ImageView; }
    VkDeviceMemory GetMemory() const { return TextureBase.Memory; }
    VkImageLayout GetImageLayout() const { return TextureBase.ImageLayout; }
    void SetImageLayout(VkImageLayout ImageLayout) { TextureBase.ImageLayout = ImageLayout; }
    VulkanTextureBase TextureBase;
};

class VulkanBaseTexture : public RHITexture
{
public:
    VulkanBaseTexture(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
    : RHITexture(InNumMips, InFormat, InTextureName)
    { }
};

class VulkanBaseTexture2D : public RHITexture2D
{
public:
    VulkanBaseTexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
    : RHITexture2D(InSizeX, InSizeY, InNumMips, InFormat, InTextureName)
    { }
};

class VulkanBaseTexture2DArray : public RHITexture2DArray
{
public:
    VulkanBaseTexture2DArray(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
    : RHITexture2DArray(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    { }
};

class VulkanBaseTexture3D : public RHITexture3D
{
public:
    VulkanBaseTexture3D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
    : RHITexture3D(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    { }
};

class VulkanBaseTextureCube : public RHITextureCube
{
public:
    VulkanBaseTextureCube(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, EPixelFormat InFormat, const std::string &InTextureName)
    : RHITextureCube(InSizeX, InNumMips, InFormat, InTextureName)
    { }
};

using VulkanTexture = TVulkanTexture<VulkanBaseTexture>;
using VulkanTexture2D = TVulkanTexture<VulkanBaseTexture2D>;
using VulkanTexture2DArray = TVulkanTexture<VulkanBaseTexture2DArray>;
using VulkanTexture3D = TVulkanTexture<VulkanBaseTexture3D>;
using VulkanTextureCube = TVulkanTexture<VulkanBaseTextureCube>;

using VulkanTextureBaseRef = std::shared_ptr<VulkanTextureBase>;
using VulkanTextureRef = std::shared_ptr<VulkanTexture>;
using VulkanTexture2DRef = std::shared_ptr<VulkanTexture2D>;
using VulkanTexture2DArrayRef = std::shared_ptr<VulkanTexture2DArray>;
using VulkanTexture3DRef = std::shared_ptr<VulkanTexture3D>;
using VulkanTextureCubeRef = std::shared_ptr<VulkanTextureCube>;

inline VulkanTextureBase* ResourceCast(RHITexture* Texture)
{
    switch (Texture->GetTextureType()) 
    {
    case ETextureType::TT_Texture2D:
    {
        VulkanTexture2D* vkTextue = static_cast<VulkanTexture2D*>(Texture);
        return &vkTextue->TextureBase;
    }
    case ETextureType::TT_Texture2DArray:
    {
        VulkanTexture2DArray* vkTextue = static_cast<VulkanTexture2DArray*>(Texture);
        return &vkTextue->TextureBase;
    }
    case ETextureType::TT_Texture3D:
    {
        VulkanTexture3D* vkTextue = static_cast<VulkanTexture3D*>(Texture);
        return &vkTextue->TextureBase;
    }
    case ETextureType::TT_TextureCube:
    {
        VulkanTextureCube* vkTextue = static_cast<VulkanTextureCube*>(Texture);
        return &vkTextue->TextureBase;
    }
    }
    return nullptr;
}

}