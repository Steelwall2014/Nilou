#pragma once
#include <string>

#include "Platform.h"
#include "RHIDefinitions.h"

namespace nilou {

class RHIResource;

class FRDGResource
{
public:
    FRDGResource() = default;
	FRDGResource(const FRDGResource&) = delete;
	virtual ~FRDGResource() = default;

    std::string Name;

    RHIResource* GetRHI() const
    {
        return ResourceRHI;
    }

protected:

    RHIResource* ResourceRHI = nullptr;

    bool bIsPersistent = false;

private:

    friend class FRenderGraph;
    friend class FRDGBuilder;
    class FRDGResourceNode* Node = nullptr;
};

struct FRDGTextureDesc
{
    uint32 SizeX;
    uint32 SizeY;
    uint32 SizeZ;
    uint32 ArraySize;
    uint32 NumMips;
    EPixelFormat Format;
    ETextureDimension TextureType;
    ETextureCreateFlags TexCreateFlags;

    bool operator==(const FRDGTextureDesc& Other) const = default;
};

class FRDGTexture : public FRDGResource
{
public:
    FRDGTexture(const FRDGTextureDesc&) { }
};
using FRDGTextureRef = std::shared_ptr<FRDGTexture>;

struct FRDGBufferDesc
{
    uint32 Stride;
    uint32 Size;
    EBufferUsageFlags Usage;

    bool operator==(const FRDGBufferDesc& Other) const = default;
};

class FRDGBuffer : public FRDGResource
{
public:
    FRDGBuffer(const FRDGBufferDesc&) { }
};
using FRDGBufferRef = std::shared_ptr<FRDGBuffer>;

struct FRDGUniformBufferDesc
{
    uint32 Size;
    EBufferUsageFlags Usage;

    bool operator==(const FRDGUniformBufferDesc& Other) const = default;
};

class FRDGUniformBuffer : public FRDGResource
{

public:

    FRDGUniformBuffer(const FRDGUniformBufferDesc& InDesc)
        : Desc(InDesc)
    {
        Data = new uint8[Desc.Size];
    }

    template <typename T>
    T& GetData()
    {
        if (sizeof(T) > Desc.Size)
            NILOU_LOG(Error, "Enough memory is not allocated in the uniform buffer. {} bytes expected but only {} bytes were allocated. ", sizeof(T), Desc.Size)
        return *static_cast<T*>(Data);
    }

private:

    FRDGUniformBufferDesc Desc;
    uint8* Data;

};
using FRDGUniformBufferRef = std::shared_ptr<FRDGUniformBuffer>;

}

namespace std {

template<>
struct hash<nilou::FRDGTextureDesc>
{
	size_t operator()(const nilou::FRDGTextureDesc &_Keyval) const noexcept;
};

template<>
struct hash<nilou::FRDGBufferDesc>
{
	size_t operator()(const nilou::FRDGBufferDesc &_Keyval) const noexcept;
};

template<>
struct hash<nilou::FRDGUniformBufferDesc>
{
	size_t operator()(const nilou::FRDGUniformBufferDesc &_Keyval) const noexcept;
};

}