#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"
#include "VulkanBarriers.h"

namespace nilou {


class VulkanTextureBase
{
public:
    VkImage Image{};
    VkImageView ImageView{};
    VkDeviceMemory Memory{};
    uint8 BaseMipLevel{};
    uint8 NumMips{};
    uint8 BaseArrayLayer{};
    uint8 NumLayers{};
    VulkanTextureBase* ParentTexture{};
    VkImageAspectFlags FullAspectFlags{};

    #ifdef NILOU_DEBUG
    FVulkanImageLayout DebugLayout;
    #endif

    VulkanTextureBase(
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        VkImageAspectFlags InFullAspectFlag, 
        const FVulkanImageLayout& InImageLayout
    );
    ~VulkanTextureBase();
    FVulkanImageLayout GetImageLayout() const;
    void SetImageLayout(VkImageLayout Layout, const VkImageSubresourceRange& Range);
    void SetFullImageLayout(VkImageLayout Layout);
    bool IsImageView() const { return ParentTexture != nullptr; }

};

template<typename BaseType>
class TVulkanTexture : public BaseType
{
public:
    TVulkanTexture(
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        const FVulkanImageLayout& InImageLayout,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InNumMips,
        EPixelFormat InFormat,
        const std::string &InTextureName
    )
    : TextureBase(InImage, InImageView, InMemory, GetFullAspectMask(InFormat), InImageLayout)
    , BaseType(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    {
        TextureBase.NumMips = InNumMips;
        TextureBase.NumLayers = BaseType::GetNumLayers();
    }
    VkImage GetImage() const { return TextureBase.Image; }
    VkImageView GetImageView() const { return TextureBase.ImageView; }
    VkDeviceMemory GetMemory() const { return TextureBase.Memory; }
    FVulkanImageLayout GetImageLayout() const { return TextureBase.GetImageLayout(); }
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

template<typename BaseType>
class TVulkanTextureView : public BaseType
{
public:
    TVulkanTextureView(
        VulkanTextureBase* InParentTexture,
        VkImage InImage,
        VkImageView InImageView,
        VkDeviceMemory InMemory,
        const FVulkanImageLayout& InImageLayout,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InBaseMipLevel,
        uint32 InNumMips,
        uint32 InBaseArrayLayer,
        uint32 InNumLayers,
        EPixelFormat InFormat,
        const std::string &InTextureName)   
        : BaseType(InImage, InImageView, InMemory, InImageLayout, InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    { 
        this->TextureBase.BaseMipLevel = InBaseMipLevel;
        this->TextureBase.BaseArrayLayer = InBaseArrayLayer;
        this->TextureBase.NumMips = InNumMips;
        this->TextureBase.NumLayers = InNumLayers;
        this->TextureBase.ParentTexture = InParentTexture;
        #ifdef NILOU_DEBUG
        this->TextureBase.DebugLayout = this->TextureBase.GetImageLayout();
        #endif
    }
};
using VulkanTextureView2D = TVulkanTextureView<VulkanTexture2D>;
using VulkanTextureViewCube = TVulkanTextureView<VulkanTextureCube>;

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