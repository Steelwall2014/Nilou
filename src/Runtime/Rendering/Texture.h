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
#include "RenderingThread.h"
#include "RenderGraphResources.h"
namespace nilou {

    enum class EImageType
    {
        IT_Image2D,
        IT_Image3D,
        IT_Image2DArray,
        IT_ImageCube
    };

	class NSTRUCT FImage 
	{
        GENERATED_STRUCT_BODY()

    public:

        FImage() { }

        FImage(
            uint32 InWidth, 
            uint32 InHeight,
            EPixelFormat InPixelFormat,
            EImageType InImageType,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, 1, 
                InPixelFormat, InMipmap, InImageType)
        { }

        FImage(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InDepth,
            EPixelFormat InPixelFormat,
            EImageType InImageType,
            uint32 InMipmap = 1)
            : FImage(
                InWidth, InHeight, InDepth, 
                InPixelFormat, InMipmap, InImageType)
        { }

        FImage(
            uint32 InWidth, 
            uint32 InHeight,
            uint32 InDepth,
            EPixelFormat InPixelFormat,
            uint32 InNumMips,
            EImageType InImageType)
            : Width(InWidth)
            , Height(InHeight)
            , Depth(InDepth)
            , PixelFormat(InPixelFormat)
            , ImageType(InImageType)
        {
            Channel = TranslatePixelFormatToChannel(PixelFormat);
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            Data.BufferSize = BytePerPixel * Width * Height * Depth;
        }

        uint32 GetWidth() const { return Width; }

        uint32 GetHeight() const { return Height; }

        uint32 GetChannel() const { return Channel; }

        uint32 GetDepth() const { return Depth; }

        uint32 GetNumLayers() const { return Depth; }

        uint64 GetDataSize() const { return Data.BufferSize; }

        uint64 GetAllocatedDataSize() const { return AllocatedDataSize; }

        uint8* GetData() { return Data.Buffer.get(); }

        EPixelFormat GetPixelFormat() const { return PixelFormat; }

        EImageType GetImageType() const { return ImageType; }

        void AllocateSpace()
        {
            Data.Buffer = std::make_shared<unsigned char[]>(Data.BufferSize);
            AllocatedDataSize = Data.BufferSize;
        }

        void ConservativeAllocateSpace()
        {
            auto NewData = std::make_shared<unsigned char[]>(Data.BufferSize);
            std::copy(Data.Buffer.get(), Data.Buffer.get()+glm::min((uint64)Data.BufferSize, AllocatedDataSize), NewData.get());
            Data.Buffer = NewData;
            AllocatedDataSize = Data.BufferSize;
        }

        void* GetPointer(int Row, int Column, int Layer)
        {
            if (Data.Buffer == nullptr)
                return nullptr;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            uint64 LayerOffset = Width * Height * Channel * Layer;
            if (Row >= Height || Column >= Width || Layer >= Depth)
                return nullptr;
            uint64 offset = (Row * Width + Column) * BytePerPixel + LayerOffset;
            if (offset >= Data.BufferSize)
                return nullptr;
            return Data.Buffer.get() + offset;
        }

        /**
         * @brief Resize the image. This function WILL NOT reallocate memory.
         * 
         * If parameter NewNumMips is not greater than zero, then the mipmap level 
         * will not be resized. Ditto for NewChannel.
         */
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewDepth, uint32 NewChannel=-1)
        {
            if (NewChannel > 0)
                Channel = NewChannel;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            Width = NewWidth;
            Height = NewHeight;
            Depth = NewDepth;
            Data.BufferSize = BytePerPixel * Width * Height * Channel * Depth;
        }

        NPROPERTY()
        uint32 Width = 0;

        NPROPERTY()
        uint32 Height = 0;

        NPROPERTY()
        uint32 Channel = 0;

        NPROPERTY()
        uint32 Depth = 0;

        NPROPERTY()
        FBinaryBuffer Data;

        NPROPERTY()
		EPixelFormat PixelFormat;

        NPROPERTY()
        EImageType ImageType;

        uint64 AllocatedDataSize = 0;
	};

    class FTexture : public FRenderResource
    {
	public:
        RDGTextureRef TextureRDG;

        RHISamplerStateRef SamplerStateRHI;

        RHISampler SamplerRHI;

        std::string Name;

        /**
         * This indicates the number of mipmaps of RHI resource, 
         * so it may be different from the image's.
         */
        int32 NumMips;

        ETextureDimension TextureType = ETextureDimension::Texture2D;

        virtual ~FTexture() { ReleaseResource(); }
		
        virtual void ReleaseRHI() override;

        /**
         * Update RHI data using the given image
         * if the given image is expired, then no update will be applied.
         * if the GetData() function of the given image returns nullptr, 
         * then the RHI still will be updated, but no data will be uploaded.
         */
        void SetData(FImage* InImage);

        RHISampler GetSamplerRHI()
        {
            return SamplerRHI;
        }

        void SetSamplerState(const FSamplerStateInitializer &InSamplerState)
        {
            SamplerStateRHI = RHICreateSamplerState(InSamplerState);
            SamplerRHI.SamplerState = SamplerStateRHI;
        }

        /** Returns the width of the texture in pixels. */
        uint32 GetSizeX() const
        {
            if (TextureRDG)
                return TextureRDG->Desc.SizeX;
            return 0;
        }
        /** Returns the height of the texture in pixels. */
        uint32 GetSizeY() const
        {
            if (TextureRDG)
                return TextureRDG->Desc.SizeY;
            return 0;
        }
        /** Returns the depth of the texture in pixels. */
        uint32 GetSizeZ() const
        {
            if (TextureRDG)
                return TextureRDG->Desc.SizeZ;
            return 0;
        }
        uint32 GetNumLayers() const
        {
            if (TextureRDG)
                return TextureRDG->Desc.ArraySize;
            return 0;
        }

        RDGTexture* GetTextureRDG() const
        {
            return TextureRDG;
        }

        RHISamplerState* GetSamplerState() const
        {
            return SamplerStateRHI;
        }

        EPixelFormat GetFormat() const
        {
            return TextureRDG->Desc.Format;
        }

    protected:

		FTexture(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : NumMips(InNumMips)
            , Name(InName)
        { 
            SetSamplerState(InSamplerState);
            SamplerRHI.Texture = nullptr;
        }

        FImage* Image;

    };

    class FTextureResource : public FTexture
    {
    public:

        FTextureResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTexture(InName, InSamplerState, InNumMips)
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

    class NCLASS UTexture : public NAsset
    {
        GENERATED_BODY()
    public:

        /**
         * We can update the texture by manipulating this ImageData, 
         * or read the texture data from the GPU.
         * For example, if we want to create a new texture or update current texture,
         * we can write it as follows:
         * 
         * std::shared_ptr<FImage2D> Image = std::make_shared<FImage2D>(1024, 1024, EPixelFormat::PF_R8G8B8A8);
         * 
         * Image->AllocateSpace();  // Allocates actual memory space for the image data
         * ****do something to the image data****
         * 
         * ****or just don't allocate the space and do nothing here****
         * 
         * // Suppose we have a pointer called Texture
         * Texture->ImageData = Image;
         * Texture->UpdateResource();
         * 
         * If we want to read data from the texture (e.g. read a render target from GPU to CPU),
         * we can do it like this:
         * 
         * Texture->ReadPixelsSync();
         * uint8* Data = Image->GetData();
         * 
         * 
         * Note: The data of the image will always be nullptr if the texture is UVirtualTexture.
         * 
         */
        NPROPERTY()
        FImage ImageData;

        /**
         * Modify the values of SamplerState and then call UpdateResource() 
         * to update sampler parameters
         */
        NPROPERTY()
        FSamplerStateInitializer SamplerState;

        /**
         * Modify the values of NumMips and then call UpdateResource() 
         * to update the number of mipmap levels
         */
        NPROPERTY()
        uint32 NumMips;

        UTexture()
            : TextureResource(nullptr)
            , TextureResourceRenderThread(nullptr)
            , NumMips(1)
        {

        }

        virtual ~UTexture()
        {
            ReleaseResource();
        }

        /**
         * Create a new UVirtualTexture object from this texture
         * Note: the content of this texture will be MOVED to the newly created virtual texture
         */
        // std::shared_ptr<class UVirtualTexture> MakeVirtualTexture();

        ETextureDimension GetTextureType() const
        {
            return TextureResource->TextureType;
        }

        /** Returns the width of the texture in pixels. */
        uint32 GetSizeX() const
        {
            if (TextureResource)
                return TextureResource->GetSizeX();
            return 0;
        }
        /** Returns the height of the texture in pixels. */
        uint32 GetSizeY() const
        {
            if (TextureResource)
                return TextureResource->GetSizeY();
            return 0;
        }
        /** Returns the depth of the texture in pixels. */
        uint32 GetSizeZ() const
        {
            if (TextureResource)
                return TextureResource->GetSizeZ();
            return 0;
        }

        /**
         * Implemented by subclasses to create a new resource for the texture.
         */
        virtual FTextureResource* CreateResource() { return nullptr; }

        void SetResource(FTextureResource* InResource)
        {
            TextureResource = InResource;
            ENQUEUE_RENDER_COMMAND(UTexture_SetResource)(
                [this, InResource](RHICommandList&) {
                    TextureResourceRenderThread = InResource;
                });
        }

        FTextureResource *GetResource()
        {
            if (IsInRenderingThread())
                return TextureResourceRenderThread;
            else
                return TextureResource;
        }

        void UpdateResource()
        {
            ReleaseResource();
            FTextureResource* NewResource = CreateResource();
            SetResource(NewResource);
            BeginInitResource(NewResource);
        }

        void ReleaseResource()
        {
            if (TextureResource)
            {
                FTextureResource* ToDelete = TextureResource;
                ENQUEUE_RENDER_COMMAND(UTexture_ReleaseResource)(
                    [ToDelete](RHICommandList&) {
                        ToDelete->ReleaseResource();
                        delete ToDelete;
                    });
            }
        }

        /**
         * Implemented by subclasses to read pixels from GPU.
         * The readed pixels will be stored in ImageData
         */
        virtual void ReadPixelsRenderThread(RHICommandList& RHICmdList) { }

        /**
         * Read pixels from GPU. It will block the thread calling it.
         * This function ensures the data of the image is readed when return.
         */
        virtual void ReadPixelsSync();

        virtual void PostDeserialize(FArchive& Ar) override;
    
    protected:

        FTextureResource* TextureResource;

        FTextureResource* TextureResourceRenderThread;

        struct ImageCreateInfo 
        {
            uint32 Width;
            uint32 Height;
            uint32 Depth;
            uint32 Channel;
            uint32 NumMips;
            EImageType ImageType;
            EPixelFormat PixelFormat;
        };
        /**
         * Implemented by subclasses to create a image.
         */
        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) { return FImage(); }

        void DeserializeImageData(FArchive& Ar);
    };

}