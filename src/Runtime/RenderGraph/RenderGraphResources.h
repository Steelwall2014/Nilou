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
class RDGTexture : public RDGResource
{
public:
    friend class RenderGraph;
    
    RDGTexture(const RDGTextureDesc& InDesc): Desc(InDesc) { }

    RHITexture* GetRHI() const { return static_cast<RHITexture*>(ResourceRHI.get()); }

    class RDGTextureView* GetDefaultView() const { return DefaultView; }

    const RDGTextureDesc Desc;

    virtual ~RDGTexture()
    {
        if (bIsPersistent)
        {
            delete DefaultView;
        }
    }

private:

    RDGTextureView* DefaultView;
};
using RDGTextureRef = std::shared_ptr<RDGTexture>;
struct RDGTextureViewDesc
{
    EPixelFormat Format; 
    uint32 BaseMipLevel;
    uint32 LevelCount;
    uint32 BaseArrayLayer;
    uint32 LayerCount;
    ETextureDimension ViewType;
    RDGTexture* Texture;

    RDGTextureViewDesc() = default;

    RDGTextureViewDesc(RDGTexture* InTexture)
    {
        Texture = InTexture;
        Format = InTexture->Desc.Format;
        BaseMipLevel = 0;
        LevelCount = InTexture->Desc.NumMips;
        BaseArrayLayer = 0;
        LayerCount = InTexture->Desc.ArraySize;
        ViewType = InTexture->Desc.TextureType;
    }

    static RDGTextureViewDesc Create(RDGTexture* InTexture)
    {
        return RDGTextureViewDesc(InTexture);
    }

    static RDGTextureViewDesc CreateForMipLevel(RDGTexture* InTexture, uint32 MipLevel)
    {
        RDGTextureViewDesc Desc = RDGTextureViewDesc::Create(InTexture);
        Desc.BaseMipLevel = MipLevel;
        Desc.LevelCount = 1;
        return Desc;
    }

    bool operator==(const RDGTextureViewDesc& Other) const = default;
};
class RDGTextureView : public RDGResource
{
public:
    RDGTextureView(std::string InName, const RDGTextureViewDesc& InDesc) 
        : RDGResource(InName)
        , Desc(InDesc)
    { }

    const RDGTextureViewDesc Desc;

    RDGTexture* GetParent() const { return Desc.Texture; }
};
using RDGTextureViewRef = std::shared_ptr<RDGTextureView>;

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

    void SetData(const void* InData)
    {
        SetData(InData, Desc.Size, 0);
    }

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

class RDGFramebuffer
{
public:

    void SetAttachment(EFramebufferAttachment Attachment, RDGTextureView* Texture);

    const RHIRenderTargetLayout& GetRenderTargetLayout() const { return RTLayout; }

private:

    RHIRenderTargetLayout RTLayout;

    std::unordered_map<EFramebufferAttachment, RDGTextureView*> Attachments;
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