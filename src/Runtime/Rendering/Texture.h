#pragma once
#include <half/half.hpp>
#include <string>
#include <thread_pool/BS_thread_pool.hpp>
#include <variant>

#include "Common/CoreUObject/Object.h"
#include "Common/LruCache.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RenderResource.h"
namespace nilou {

    enum class EImageType
    {
        IT_Image2D,
        IT_Image3D,
        IT_Image2DArray,
        IT_ImageCube
    };

	class FImage 
	{
    public:

        uint32 GetWidth() const { return Width; }

        uint32 GetHeight() const { return Height; }

        uint32 GetChannel() const { return Channel; }

        uint32 GetDepth() const { return Depth; }

        uint32 GetLayer() const { return Depth; }

        uint32 GetMipmap() const { return Mipmap; }

        uint64 GetDataSize() const { return DataSize; }

        uint64 GetActualDataSize() const { return ActualDataSize; }

        uint8* GetData() { return Data.get(); }

        EPixelFormat GetPixelFormat() const { return PixelFormat; }

        EImageType GetImageType() const { return ImageType; }

        void AllocateSpace()
        {
            Data = std::make_unique<unsigned char[]>(DataSize);
            ActualDataSize = DataSize;
        }

        void ConservativeAllocateSpace()
        {
            auto NewData = std::make_unique<unsigned char[]>(DataSize);
            std::copy(Data.get(), Data.get()+glm::min(DataSize, ActualDataSize), NewData.get());
            Data = std::move(NewData);
            ActualDataSize = DataSize;
        }

	protected:
        FImage(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InChannel,
            uint32 InDepth,
            EPixelFormat InPixelFormat,
            uint32 InMipmap,
            EImageType InImageType)
            : Width(InWidth)
            , Height(InHeight)
            , Channel(InChannel)
            , Depth(InDepth)
            , PixelFormat(InPixelFormat)
            , Mipmap(InMipmap)
            , ImageType(InImageType)
        {
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            DataSize = BytePerPixel * Width * Height * Depth;
            DataSize = DataSize * (1.0 - pow(0.25, Mipmap)) / (1.0 - 0.25);
        }

        void* GetPointer(int row, int col, int layer, int mipmap=0)
        {
            if (Data == nullptr)
                return nullptr;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            uint64 MipmapOffset = BytePerPixel * Width * Height * Channel * Depth;
            MipmapOffset = MipmapOffset * (1.0 - pow(0.25, Mipmap)) / (1.0 - 0.25);
            uint64 LayerOffset = Width * Height * Channel * layer;
            int mip_width = Width / glm::pow(2, mipmap);
            uint64 offset = (row * mip_width + col) * BytePerPixel + LayerOffset + MipmapOffset;
            if (offset >= DataSize)
                return nullptr;
            return Data.get() + offset;
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewMipmap is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewDepth, uint32 NewChannel=-1, int32 NewMipmap=-1)
        {
            if (NewMipmap > 0)
                Mipmap = NewMipmap;
            if (NewChannel > 0)
                Channel = NewChannel;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            Width = NewWidth;
            Height = NewHeight;
            Depth = NewDepth;
            DataSize = BytePerPixel * Width * Height * Channel * Depth;
            DataSize = DataSize * (1.0 - pow(0.25, Mipmap)) / (1.0 - 0.25);
        }

        uint32 Width = 0;
        uint32 Height = 0;
        uint32 Channel = 0;
        uint32 Depth = 0;
        uint32 Mipmap = 0;
        std::unique_ptr<unsigned char[]> Data = nullptr;
        uint64 DataSize = 0;
        uint64 ActualDataSize = 0;
		EPixelFormat PixelFormat;
        EImageType ImageType;
	};

    class FImage2D : public FImage
    {
    public:

        FImage2D(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InChannel,
            EPixelFormat InPixelFormat,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, InChannel, 1, 
                InPixelFormat, InMipmap, EImageType::IT_Image2D)
        {

        }

        void* GetPointer(int row, int col, int mipmap=0)
        {
            return FImage::GetPointer(row, col, 0, mipmap);
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewMipmap is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewChannel=-1, int32 NewMipmap=-1)
        {
            FImage::Resize(NewWidth, NewHeight, 1, NewChannel, NewMipmap);
        }

    };

    class FImage3D : public FImage
    {
    public:

        FImage3D(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InChannel,
            uint32 InDepth,
            EPixelFormat InPixelFormat,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, InChannel, InDepth, 
                InPixelFormat, InMipmap, EImageType::IT_Image3D)
        {

        }
        
        void* GetPointer(int row, int col, int depth, int mipmap=0)
        {
            return FImage::GetPointer(row, col, depth, mipmap);
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewMipmap is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewDepth, uint32 NewChannel=-1, int32 NewMipmap=-1)
        {
            FImage::Resize(NewWidth, NewHeight, NewDepth, NewChannel, NewMipmap);
        }

    };

    class FImage2DArray : public FImage
    {
    public:

        FImage2DArray(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InChannel,
            uint32 InLayer,
            EPixelFormat InPixelFormat,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, InChannel, InLayer, 
                InPixelFormat, InMipmap, EImageType::IT_Image2DArray)
        {

        }
        
        void* GetPointer(int row, int col, int layer, int mipmap=0)
        {
            return FImage::GetPointer(row, col, layer, mipmap);
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewMipmap is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewLayer, uint32 NewChannel=-1, int32 NewMipmap=-1)
        {
            FImage::Resize(NewWidth, NewHeight, NewLayer, NewChannel, NewMipmap);
        }

    };

    class FImageCube : public FImage
    {
    public:

        FImageCube(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InChannel,
            EPixelFormat InPixelFormat,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, InChannel, 6, 
                InPixelFormat, InMipmap, EImageType::IT_ImageCube)
        {

        }
        
        void* GetPointer(int row, int col, int layer, int mipmap=0)
        {
            return FImage::GetPointer(row, col, layer, mipmap);
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewMipmap is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewChannel=-1, int32 NewMipmap=-1)
        {
            FImage::Resize(NewWidth, NewHeight, 1, NewChannel, NewMipmap);
        }

    };

    class FTexture : public FRenderResource
    {
	public:
        RHITextureRef TextureRHI;

        FRHISampler SamplerRHI;

        std::string Name;

        /**
         * This indicates the number of mipmaps of RHI resource, 
         * so it may be different from the image's.
         */
        int32 NumMips;

        ETextureType TextureType = ETextureType::TT_Texture2D;

        virtual ~FTexture() { ReleaseResource(); }
		
        virtual void InitRHI() override;
        virtual void ReleaseRHI() override;

        /**
         * Update RHI data using the given image
         * if the given image is expired, then no update will be applied.
         * if the GetData() function of the given image returns nullptr, 
         * then the RHI still will be updated, but no data will be uploaded.
         */
        void SetData(std::weak_ptr<FImage> InImage);

        FRHISampler *GetSamplerRHI()
        {
            return &SamplerRHI;
        }

        void SetSamplerParams(const RHITextureParams &InTextureParams)
        {
            SamplerRHI.Params = InTextureParams;
        }

        /** Returns the width of the texture in pixels. */
        uint32 GetSizeX() const
        {
            if (TextureRHI)
                return TextureRHI->GetSizeXYZ().x;
            return 0;
        }
        /** Returns the height of the texture in pixels. */
        uint32 GetSizeY() const
        {
            if (TextureRHI)
                return TextureRHI->GetSizeXYZ().y;
            return 0;
        }
        /** Returns the depth of the texture in pixels. */
        uint32 GetSizeZ() const
        {
            if (TextureRHI)
                return TextureRHI->GetSizeXYZ().z;
            return 0;
        }

    protected:

		FTexture(const std::string& InName, int32 InNumMips=1)
            : NumMips(InNumMips)
            , Name(InName)
        { }

        std::weak_ptr<FImage> WeakImage;

    };

    class FTextureResource : virtual public FTexture
    {
    public:

        FTextureResource(const std::string& InName, int32 InNumMips=1)
            : FTexture(InName, InNumMips)
        { }

	    // Dynamic cast methods.
	    virtual class FTexture2DResource* GetTexture2DResource() { return nullptr; }
	    virtual class FTexture3DResource* GetTexture3DResource() { return nullptr; }
	    virtual class FTexture2DArrayResource* GetTexture2DArrayResource() { return nullptr; }
	    virtual class FTextureCubeResource* GetTextureCubeResource() { return nullptr; }
	    virtual class FVirtualTexture2DResource* GetVirtualTexture2DResource() { return nullptr; }
	    // Dynamic cast methods (const).
	    virtual const FTexture2DResource* GetTexture2DResource() const { return nullptr; }
	    virtual const FTexture3DResource* GetTexture3DResource() const { return nullptr; }
	    virtual const FTexture2DArrayResource* GetTexture2DArrayResource() const { return nullptr; }
	    virtual const FTextureCubeResource* GetTextureCubeResource() const { return nullptr; }
	    virtual const FVirtualTexture2DResource* GetVirtualTexture2DResource() const { return nullptr; }
        

    };

    class FTexture2DResource : public FTextureResource
    {
    public:

        FTexture2DResource(const std::string& InName, int32 InNumMips=1)
            : FTextureResource(InName, InNumMips)
            , FTexture(InName, InNumMips)
        { }

        virtual void InitRHI() override;

	    virtual FTexture2DResource* GetTexture2DResource() override { return this; }
	    virtual const FTexture2DResource* GetTexture2DResource() const override { return this; }
    };

    class FTexture3DResource : public FTextureResource
    {
    public:

        FTexture3DResource(const std::string& InName, int32 InNumMips=1)
            : FTextureResource(InName, InNumMips)
            , FTexture(InName, InNumMips)
        { }

        virtual void InitRHI() override;

	    virtual FTexture3DResource* GetTexture3DResource() override { return this; }
	    virtual const FTexture3DResource* GetTexture3DResource() const override { return this; }
    };

    class FTexture2DArrayResource : public FTextureResource
    {
    public:

        FTexture2DArrayResource(const std::string& InName, int32 InNumMips=1)
            : FTextureResource(InName, InNumMips)
            , FTexture(InName, InNumMips)
        { }

        virtual void InitRHI() override;

	    virtual FTexture2DArrayResource* GetTexture2DArrayResource() override { return this; }
	    virtual const FTexture2DArrayResource* GetTexture2DArrayResource() const override { return this; }
    };

    class FTextureCubeResource : public FTextureResource
    {
    public:

        FTextureCubeResource(const std::string& InName, int32 InNumMips=1)
            : FTextureResource(InName, InNumMips)
            , FTexture(InName, InNumMips)
        { }

        virtual void InitRHI() override;

	    virtual FTextureCubeResource* GetTextureCubeResource() override { return this; }
	    virtual const FTextureCubeResource* GetTextureCubeResource() const override { return this; }
    };

    class FVirtualTexture2DResource : public FTextureResource
    {
	public:

        FVirtualTexture2DResource(const std::string& InName, int32 InNumMips=1)
            : FTextureResource(InName, InNumMips)
            , FTexture(InName, InNumMips)
        { }
		
        virtual void InitRHI() override;

	    virtual FVirtualTexture2DResource* GetVirtualTexture2DResource() override { return this; }
	    virtual const FVirtualTexture2DResource* GetVirtualTexture2DResource() const override { return this; }
    };

    class FTextureRenderTargetResource : virtual public FTexture
    {
	public:
        
        virtual class FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() { return nullptr; }
        virtual class FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() { return nullptr; }

    };

    class FTextureRenderTarget2DResource : public FTextureRenderTargetResource, public FTexture2DResource
    {
	public:
		
        virtual void InitRHI() override;
        
        virtual FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() { return this; }

    protected:

        RHIFramebufferRef Framebuffer;

        RHITexture2DRef DepthStencilRHI;
    };

    class FTextureRenderTargetCubeResource : public FTextureRenderTargetResource, public FTextureCubeResource
    {
	public:
		
        virtual void InitRHI() override;
        
        virtual FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() { return this; }

    protected:

        std::array<RHIFramebufferRef, 6> Framebuffers;

        std::array<RHITexture2DRef, 6> DepthStencilRHIs;

        std::array<RHITexture2DRef, 6> FaceRenderTargets;

    };

    UCLASS()
    class UTexture : public UObject
    {
        GENERATE_CLASS_INFO()
    public:

		std::string Name;

        UTexture(const std::string &InName="", std::unique_ptr<FTextureResource> InTextureResource=nullptr)
            : Name(InName)
            , TextureResource(std::move(InTextureResource))
        {

        }

        /**
         * Create a new UVirtualTexture object from this texture
         * Note: the content of this texture will be MOVED to the newly created virtual texture
         */
        std::shared_ptr<class UVirtualTexture> MakeVirtualTexture();

        RHITextureParams GetTextureParams() const
        {
            return TextureResource->GetSamplerRHI()->Params;
        }

        ETextureType GetTextureType() const
        {
            return TextureResource->TextureType;
        }

        FTexture *GetResource()
        {
            return TextureResource.get();
        }

        void SetData(std::shared_ptr<FImage> InImage)
        {
            Image = InImage;
            if (TextureResource)
                TextureResource->SetData(InImage);
        }

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        virtual void ReleaseRenderResources()
        {
            BeginReleaseResource(TextureResource.get());
        }
    
    protected:

        void DeserializeWithoutImageData(FArchive &Ar);

        std::unique_ptr<FTexture> TextureResource;

        std::shared_ptr<FImage> Image;
    };

    struct VirtualTextureTile
    {
        uint32 TileX;
        uint32 TileY;
        uint32 MipmapLevel;
        uint32 DataOffset;
        bool bCommited = false;
        std::mutex mutex;
    };

    UCLASS()
    class UVirtualTexture : public UTexture
    {
        GENERATE_CLASS_INFO()
    public:
        UVirtualTexture();

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        void UpdateBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UpdateBoundSync(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTileSync(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        uvec2 GetNumTiles(int MipmapLevel=0) const { return uvec2(NumTileX >> MipmapLevel, NumTileY >> MipmapLevel); }

        ivec3 GetPageSize() const { return PageSize; }

        uint32 GetBytePerTile() const { return BytePerTile; }

        uint32 MaxPhysicalMemoryByte = 1024*1024*128;   // 128 MB physical memory limit for every virtual texture

   private:

        void UpdateTileInternal(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        uint32 NumTileX;
        uint32 NumTileY;

        ivec3 PageSize;

        uint32 BytePerTile;

        std::vector<std::vector<std::vector<std::unique_ptr<VirtualTextureTile>>>> Tiles;

        TLruCache<VirtualTextureTile*, VirtualTextureTile*> LruCache;

        std::filesystem::path StreamingPath;
        // It's the offset from the beginning to the BIN block
        uint32 StreamingBufferOffset;

        BS::thread_pool thread_pool;

    };

    UCLASS()
    class UTextureRenderTarget : public UTexture
    {
        GENERATE_CLASS_INFO()
    public:

        FTextureRenderTargetResource* GetRenderTargetResource();

    };

    UCLASS()
    class UTextureRenderTarget2D : public UTextureRenderTarget
    {
        GENERATE_CLASS_INFO()
    public:

        FTextureRenderTargetResource* GetRenderTargetResource();

        vec3 ClearColor;

    };

    UCLASS()
    class UTextureRenderTargetCube : public UTextureRenderTarget
    {
        GENERATE_CLASS_INFO()
    public:



    };

}