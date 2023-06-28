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
            , NumMips(InNumMips)
            , ImageType(InImageType)
        {
            Channel = TranslatePixelFormatToChannel(PixelFormat);
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            Data.BufferSize = BytePerPixel * Width * Height * Depth;
            Data.BufferSize = Data.BufferSize * (1.0 - pow(0.25, NumMips)) / (1.0 - 0.25);
        }

        uint32 GetWidth() const { return Width; }

        uint32 GetHeight() const { return Height; }

        uint32 GetChannel() const { return Channel; }

        uint32 GetDepth() const { return Depth; }

        uint32 GetNumLayers() const { return Depth; }

        uint32 GetNumMips() const { return NumMips; }

        uint64 GetDataSize() const { return Data.BufferSize; }

        uint64 GetActualDataSize() const { return ActualDataSize; }

        uint8* GetData() { return Data.Buffer.get(); }

        EPixelFormat GetPixelFormat() const { return PixelFormat; }

        EImageType GetImageType() const { return ImageType; }

        void AllocateSpace()
        {
            Data.Buffer = std::make_shared<unsigned char[]>(Data.BufferSize);
            ActualDataSize = Data.BufferSize;
        }

        void ConservativeAllocateSpace()
        {
            auto NewData = std::make_shared<unsigned char[]>(Data.BufferSize);
            std::copy(Data.Buffer.get(), Data.Buffer.get()+glm::min((uint64)Data.BufferSize, ActualDataSize), NewData.get());
            Data.Buffer = NewData;
            ActualDataSize = Data.BufferSize;
        }

        void* GetPointer(int Row, int Column, int Layer, int MipIndex=0)
        {
            if (Data.Buffer == nullptr)
                return nullptr;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            uint64 MipmapOffset = BytePerPixel * Width * Height * Channel * Depth;
            MipmapOffset = MipmapOffset * (1.0 - pow(0.25, MipIndex)) / (1.0 - 0.25);
            uint64 LayerOffset = Width * Height * Channel * Layer;
            int mip_width = Width >> MipIndex;
            int mip_height = Height >> MipIndex;
            if (Row >= mip_height || Column >= mip_width || Layer >= Depth)
                return nullptr;
            uint64 offset = (Row * mip_width + Column) * BytePerPixel + LayerOffset + MipmapOffset;
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
        void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewDepth, uint32 NewChannel=-1, int32 NewNumMips=-1)
        {
            if (NewNumMips > 0)
                NumMips = NewNumMips;
            if (NewChannel > 0)
                Channel = NewChannel;
            int BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
            Width = NewWidth;
            Height = NewHeight;
            Depth = NewDepth;
            Data.BufferSize = BytePerPixel * Width * Height * Channel * Depth;
            Data.BufferSize = Data.BufferSize * (1.0 - pow(0.25, NumMips)) / (1.0 - 0.25);
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
        uint32 NumMips = 0;

        NPROPERTY()
        FBinaryBuffer Data;

        NPROPERTY()
		EPixelFormat PixelFormat;

        NPROPERTY()
        EImageType ImageType;

        uint64 ActualDataSize = 0;
	};

    // class FImage2D : public FImage
    // {
    // public:

    //     FImage2D(
    //         uint32 InWidth, 
    //         uint32 InHeight,
    //         EPixelFormat InPixelFormat,
    //         uint32 InMipmap = 1)
    //         : FImage(
    //             InWidth, InHeight, 1, 
    //             InPixelFormat, InMipmap, EImageType::IT_Image2D)
    //     {

    //     }

    //     void* GetPointer(int Row, int Column, int MipIndex=0)
    //     {
    //         return FImage::GetPointer(Row, Column, 0, MipIndex);
    //     }

    //     /**
    //      * @brief Resize the image. This function WILL NOT reallocate memory.
    //      * 
    //      * If parameter NewMipmap is not greater than zero, then the mipmap level 
    //      * will not be resized. Ditto for NewChannel.
    //      */
    //     void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewChannel=-1, int32 NewMipmap=-1)
    //     {
    //         FImage::Resize(NewWidth, NewHeight, 1, NewChannel, NewMipmap);
    //     }

    // };

    // class FImage3D : public FImage
    // {
    // public:

    //     FImage3D(
    //         uint32 InWidth, 
    //         uint32 InHeight,
    //         uint32 InDepth,
    //         EPixelFormat InPixelFormat,
    //         uint32 InMipmap = 1)
    //         : FImage(
    //             InWidth, InHeight, InDepth, 
    //             InPixelFormat, InMipmap, EImageType::IT_Image3D)
    //     {

    //     }
        
    //     void* GetPointer(int Row, int Column, int Depth, int MipIndex=0)
    //     {
    //         return FImage::GetPointer(Row, Column, Depth, MipIndex);
    //     }

    //     /**
    //      * @brief Resize the image. This function WILL NOT reallocate memory.
    //      * 
    //      * If parameter NewMipmap is not greater than zero, then the mipmap level 
    //      * will not be resized. Ditto for NewChannel.
    //      */
    //     void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewDepth, uint32 NewChannel=-1, int32 NewMipmap=-1)
    //     {
    //         FImage::Resize(NewWidth, NewHeight, NewDepth, NewChannel, NewMipmap);
    //     }

    // };

    // class FImage2DArray : public FImage
    // {
    // public:

    //     FImage2DArray(
    //         uint32 InWidth, 
    //         uint32 InHeight,
    //         uint32 InLayer,
    //         EPixelFormat InPixelFormat,
    //         uint32 InMipmap = 1)
    //         : FImage(
    //             InWidth, InHeight, InLayer,  
    //             InPixelFormat, InMipmap, EImageType::IT_Image2DArray)
    //     {

    //     }
        
    //     void* GetPointer(int Row, int Column, int Layer, int MipIndex=0)
    //     {
    //         return FImage::GetPointer(Row, Column, Layer, MipIndex);
    //     }

    //     /**
    //      * @brief Resize the image. This function WILL NOT reallocate memory.
    //      * 
    //      * If parameter NewMipmap is not greater than zero, then the mipmap level 
    //      * will not be resized. Ditto for NewChannel.
    //      */
    //     void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewLayer, uint32 NewChannel=-1, int32 NewMipmap=-1)
    //     {
    //         FImage::Resize(NewWidth, NewHeight, NewLayer, NewChannel, NewMipmap);
    //     }

    // };

    // class FImageCube : public FImage
    // {
    // public:

    //     FImageCube(
    //         uint32 InWidth, 
    //         uint32 InHeight,
    //         EPixelFormat InPixelFormat,
    //         uint32 InMipmap = 1)
    //         : FImage(
    //             InWidth, InHeight, 6, 
    //             InPixelFormat, InMipmap, EImageType::IT_ImageCube)
    //     {

    //     }
        
    //     void* GetPointer(int Row, int Column, int Layer, int MipIndex=0)
    //     {
    //         return FImage::GetPointer(Row, Column, Layer, MipIndex);
    //     }

    //     /**
    //      * @brief Resize the image. This function WILL NOT reallocate memory.
    //      * 
    //      * If parameter NewMipmap is not greater than zero, then the mipmap level 
    //      * will not be resized. Ditto for NewChannel.
    //      */
    //     void Resize(uint32 NewWidth, uint32 NewHeight, uint32 NewChannel=-1, int32 NewMipmap=-1)
    //     {
    //         FImage::Resize(NewWidth, NewHeight, 1, NewChannel, NewMipmap);
    //     }

    // };

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
        void SetData(FImage* InImage);

        FRHISampler *GetSamplerRHI()
        {
            return &SamplerRHI;
        }

        void SetSamplerParams(const RHITextureParams &InTextureParams)
        {
            SamplerRHI.SamplerState = GetSamplerStateRHI(InTextureParams);
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

		FTexture(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : NumMips(InNumMips)
            , Name(InName)
        { 
            SamplerRHI.SamplerState = GetSamplerStateRHI(InTextureParams);
            SamplerRHI.Texture = nullptr;
        }

        RHISamplerState* GetSamplerStateRHI(const RHITextureParams& InTextureParams)
        {
            return FDynamicRHI::GetDynamicRHI()->RHICreateSamplerState(InTextureParams).get();
        }

        FImage* Image;

    };

    class FTextureResource : public FTexture
    {
    public:

        FTextureResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTexture(InName, InTextureParams, InNumMips)
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

        NPROPERTY()
		std::string Name;

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
         * Modify the values of TextureParams and then call UpdateResource() 
         * to update sampler parameters
         */
        NPROPERTY()
        RHITextureParams TextureParams;

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

        /**
         * Create a new UVirtualTexture object from this texture
         * Note: the content of this texture will be MOVED to the newly created virtual texture
         */
        std::shared_ptr<class UVirtualTexture> MakeVirtualTexture();

        ETextureType GetTextureType() const
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
        uint32 GetImageNumMips()
        {
            return ImageData.GetNumMips();
        }

        /**
         * Implemented by subclasses to create a new resource for the texture.
         */
        virtual FTextureResource* CreateResource() { return nullptr; }

        void SetResource(FTextureResource* InResource)
        {
            TextureResource = InResource;
            ENQUEUE_RENDER_COMMAND(UTexture_SetResource)(
                [this, InResource](FDynamicRHI*) {
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
                    [ToDelete](FDynamicRHI*) {
                        ToDelete->ReleaseResource();
                        delete ToDelete;
                    });
            }
        }

        /**
         * Implemented by subclasses to read pixels from GPU.
         * The readed pixels will be stored in ImageData
         */
        virtual void ReadPixelsRenderThread(FDynamicRHI* RHICmdList) { }

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