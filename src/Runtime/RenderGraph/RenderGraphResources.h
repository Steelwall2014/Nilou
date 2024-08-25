#pragma once
#include <string>

#include "Platform.h"
#include "RHIResources.h"

namespace nilou {

class RDGResource
{
public:
    RDGResource() = default;
    RDGResource(std::string InName) : Name(InName) { }
	RDGResource(const RDGResource&) = delete;
	virtual ~RDGResource() = default;

    RHIResource* GetRHI() const { return ResourceRHI.get(); }

    std::string Name;
    
    bool bIsPersistent = false;

protected:

    std::shared_ptr<RHIResource> ResourceRHI = nullptr;

private:

    friend class RenderGraph;
    friend class RDGBuilder;
    class RDGResourceNode* Node = nullptr;
};

/** The set of concrete parent resource types. */
enum class ERDGViewableResourceType : uint8
{
	Texture,
	Buffer,
	MAX
};

/** The set of concrete view types. */
enum class ERDGViewType : uint8
{
	TextureUAV,
	TextureSRV,
	BufferUAV,
	BufferSRV,
	MAX
};

inline ERDGViewableResourceType GetParentType(ERDGViewType ViewType)
{
	switch (ViewType)
	{
	case ERDGViewType::TextureUAV:
	case ERDGViewType::TextureSRV:
		return ERDGViewableResourceType::Texture;
	case ERDGViewType::BufferUAV:
	case ERDGViewType::BufferSRV:
		return ERDGViewableResourceType::Buffer;
	}
    Ncheckf(false, "Invalid view type");
	return ERDGViewableResourceType::MAX;
}

class RDGViewableResource : public RDGResource
{
public:
    ERDGViewableResourceType Type;
};

class RDGView : public RDGResource
{
public:
    ERDGViewType Type;

    virtual RDGViewableResource* GetParent() const = 0;

    ERDGViewableResourceType GetParentType() const
    {
        return nilou::GetParentType(Type);
    }

protected:
    RDGView(std::string InName, ERDGViewType InType) 
        : RDGResource(InName)
        , Type(InType) 
    { }
};

class RDGShaderResourceView : public RDGView
{
protected:
    RDGShaderResourceView(std::string InName, ERDGViewType InType) 
        : RDGView(InName, InType) 
    { }
};

class RDGUnorderedAccessView : public RDGView
{
protected:
    RDGUnorderedAccessView(std::string InName, ERDGViewType InType) 
        : RDGView(InName, InType) 
    { }
};


/************ Texture *************/
struct RDGTextureDesc
{
    uint32 SizeX;
    uint32 SizeY;
    uint32 SizeZ;
    uint32 ArraySize;
    uint32 NumMips;
    EPixelFormat Format;
    ETextureDimension TextureType;
    ETextureCreateFlags TexCreateFlags;

    bool operator==(const RDGTextureDesc& Other) const = default;
};
class RDGTexture : public RDGViewableResource
{
public:
    RDGTexture(const RDGTextureDesc& InDesc): Desc(InDesc) { }

    RHITexture* GetRHI() const { return static_cast<RHITexture*>(ResourceRHI.get()); }

    const RDGTextureDesc Desc;
};
using RDGTextureRef = std::shared_ptr<RDGTexture>;

/** Texture Shader Resource View */
struct RDGTextureSRVDesc
{
    EPixelFormat Format; 
    uint32 BaseMipLevel;
    uint32 LevelCount;
    uint32 BaseArrayLayer;
    uint32 LayerCount;
    ETextureDimension ViewType;
    RDGTexture* Texture;

    RDGTextureSRVDesc() = default;

    RDGTextureSRVDesc(RDGTexture* InTexture)
    {
        Texture = InTexture;
        Format = InTexture->Desc.Format;
        BaseMipLevel = 0;
        LevelCount = InTexture->Desc.NumMips;
        BaseArrayLayer = 0;
        LayerCount = InTexture->Desc.ArraySize;
        ViewType = InTexture->Desc.TextureType;
    }

    static RDGTextureSRVDesc Create(RDGTexture* InTexture)
    {
        return RDGTextureSRVDesc(InTexture);
    }

    static RDGTextureSRVDesc CreateForMipLevel(RDGTexture* InTexture, uint32 MipLevel)
    {
        RDGTextureSRVDesc Desc = RDGTextureSRVDesc::Create(InTexture);
        Desc.BaseMipLevel = MipLevel;
        Desc.LevelCount = 1;
        return Desc;
    }

    bool operator==(const RDGTextureSRVDesc& Other) const = default;
};
class RDGTextureSRV : public RDGShaderResourceView
{
public:
    RDGTextureSRV(std::string InName, const RDGTextureSRVDesc& InDesc) 
        : RDGShaderResourceView(InName, ERDGViewType::TextureSRV)
        , Desc(InDesc) 
    { }

    const RDGTextureSRVDesc Desc;

    RDGTexture* GetParent() const { return Desc.Texture; }

};

/** Texture Unordered Access View */
struct RDGTextureUAVDesc
{
    EPixelFormat Format; 
    uint32 BaseMipLevel;
    uint32 BaseArrayLayer;
    uint32 LayerCount;
    RDGTexture* Texture;

    RDGTextureUAVDesc() = default;

    explicit RDGTextureUAVDesc(RDGTexture* InTexture, uint8 InMipLevel, EPixelFormat InFormat = PF_Unknown, uint16 InBaseArrayLayer = 0, uint16 InLayerCount = 0)
    {
        Texture = InTexture;
        BaseMipLevel = InMipLevel;
        Format = InFormat != PF_Unknown ? InFormat : InTexture->Desc.Format;
        BaseArrayLayer = InBaseArrayLayer;
        LayerCount = InLayerCount;
    }

    bool operator==(const RDGTextureUAVDesc& Other) const = default;
};
class RDGTextureUAV : public RDGUnorderedAccessView
{
public:
    RDGTextureUAV(std::string InName, const RDGTextureSRVDesc& InDesc) 
        : RDGUnorderedAccessView(InName, ERDGViewType::TextureUAV)
        , Desc(InDesc) 
    { }

    const RDGTextureSRVDesc Desc;

    RDGTexture* GetParent() const { return Desc.Texture; }
};

/************ Buffer *************/
struct RDGBufferDesc
{
    uint32 Size;
    uint32 Stride;
    // EBufferUsageFlags Usage;

    RDGBufferDesc() = default;
    RDGBufferDesc(uint32 InSize, uint32 InStride=0) : Size(InSize), Stride(InStride) { }

    bool operator==(const RDGBufferDesc& Other) const = default;
};
class RDGBuffer : public RDGViewableResource
{
public:
    RDGBuffer(const RDGBufferDesc& InDesc) 
        : Desc(InDesc) 
    { 
        Data = std::make_unique<uint8[]>(InDesc.Size);
        AllocatedSize = InDesc.Size;
    }
    RHIBuffer* GetRHI() const { return static_cast<RHIBuffer*>(ResourceRHI.get()); }

    template<typename T>
    void SetData(const T& InData, uint32 Offset)
    {
        SetData(&InData, sizeof(InData), Offset);
    }

    void SetData(const void* InData, uint32 Size, uint32 Offset)
    {
        Ncheckf(Offset+Size <= AllocatedSize, "Data size is too large, expected %d, got %d", AllocatedSize, Offset+Size);
        if (Data)
        {
            if (memcmp(Data.get(), InData, Size) != 0)
            {
                memcpy(Data.get()+Offset, InData, Size);
                bDirty = true;
            }
        }
    }

    template<typename T>
    T* GetData()
    {
        return reinterpret_cast<T*>(Data.get());
    }

    const RDGBufferDesc Desc;
    std::unique_ptr<uint8[]> Data = nullptr;
    uint32 AllocatedSize = 0;
    bool bDirty = false;
};
using RDGBufferRef = std::shared_ptr<RDGBuffer>;

/** Buffer Shader Resource View */
struct RDGBufferSRVDesc
{
	explicit RDGBufferSRVDesc() = default;

	explicit RDGBufferSRVDesc(RDGBuffer* InBuffer, EPixelFormat InFormat)
		: Format(InFormat)
        , Buffer(InBuffer)
	{
		if (InFormat != PF_Unknown)
		{
			BytesPerElement = GPixelFormats[Format].BlockBytes;
		}
	}
	/** Number of bytes per element. */
	uint32 BytesPerElement = 1;

	/** Encoding format for the element. */
	EPixelFormat Format = PF_Unknown;

    RDGBuffer* Buffer = nullptr;
};
class RDGBufferSRV : public RDGShaderResourceView
{
public:
    RDGBufferSRV(std::string InName, const RDGBufferSRVDesc& InDesc) 
        : RDGShaderResourceView(InName, ERDGViewType::BufferSRV)
        , Desc(InDesc) 
    { }

    const RDGBufferSRVDesc Desc;

    RDGBuffer* GetParent() const { return static_cast<RDGBuffer*>(Desc.Buffer); }
};

/** Buffer Unordered Access View */
struct RDGBufferUAVDesc
{
    explicit RDGBufferUAVDesc() = default;

    explicit RDGBufferUAVDesc(EPixelFormat InFormat)
        : Format(InFormat)
    { }

    /** Encoding format for the element. */
    EPixelFormat Format = PF_Unknown;

    RDGBuffer* Buffer = nullptr;
};
class RDGBufferUAV : public RDGUnorderedAccessView
{
public:
    RDGBufferUAV(std::string InName, const RDGBufferUAVDesc& InDesc) 
        : RDGUnorderedAccessView(InName, ERDGViewType::BufferUAV)
        , Desc(InDesc) 
    { }

    const RDGBufferUAVDesc Desc;

    RDGBuffer* GetParent() const { return static_cast<RDGBuffer*>(Desc.Buffer); }
};

}

namespace std {

template<>
struct hash<nilou::RDGTextureDesc>
{
	size_t operator()(const nilou::RDGTextureDesc &_Keyval) const noexcept;
};

template<>
struct hash<nilou::RDGBufferDesc>
{
	size_t operator()(const nilou::RDGBufferDesc &_Keyval) const noexcept;
};

// template<>
// struct hash<nilou::RDGUniformBufferDesc>
// {
// 	size_t operator()(const nilou::RDGUniformBufferDesc &_Keyval) const noexcept;
// };

}