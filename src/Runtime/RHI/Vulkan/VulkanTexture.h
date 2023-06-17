#pragma once
#include <vulkan/vulkan.h>
#include "RHIResources.h"

namespace nilou {


class VulkanTextureBase
{
public:
    VkImage Image;
    VkDeviceMemory Memory;

    VulkanTextureBase(
        VkImage InImage,
        VkDeviceMemory InMemory
    )
    : Image(InImage)
    , Memory(InMemory)
    { }
    virtual ~VulkanTextureBase();

};

template<typename BaseType>
class TVulkanTexture : public BaseType, public VulkanTextureBase
{
public:
    TVulkanTexture(
        VkImage InImage,
        VkDeviceMemory InMemory,
        uint32 InSizeX,
        uint32 InSizeY,
        uint32 InSizeZ,
        uint32 InNumMips,
        EPixelFormat InFormat,
        const std::string &InTextureName
    )
    : VulkanTextureBase(InImage, InMemory)
    , BaseType(InSizeX, InSizeY, InSizeZ, InNumMips, InFormat, InTextureName)
    {}
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

}