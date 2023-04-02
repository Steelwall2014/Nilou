#include "TextureCube.h"

namespace nilou {

    void FTextureCubeResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_ImageCube)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureCubeResource "
                "which requires the type of the image to be IT_ImageCube, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();
        void* data_pointers[6];
        auto CubeImage = static_cast<FImageCube*>(Image);
        for (int i = 0; i < 6; i++)
            data_pointers[i] = CubeImage->GetPointer(0, 0, i);
        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();

        FTextureResource::InitRHI();
        auto TextureCubeRHI = RHICmdList->RHICreateTextureCube(
            Name, Image->GetPixelFormat(), 
            NumMips, Image->GetWidth(), Image->GetHeight());
        TextureRHI = TextureCubeRHI;
        
        for (int i = 0; i < 6; i++)
        {
            for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
            {
                RHICmdList->RHIUpdateTextureCube(TextureCubeRHI.get(), 
                    0, 0, i, 
                    Image->GetWidth(), Image->GetHeight(), MipIndex, 
                    Image->GetPointer(0, 0, i, MipIndex));
            }
        }
        SamplerRHI.Texture = TextureRHI.get();

        RHIGetError();
    }

    FTextureResource* UTextureCube::CreateResource()
    {
        FTextureCubeResource* Resource = new FTextureCubeResource(Name, TextureParams, NumMips);
        Resource->SetData(ImageData.get());
        return Resource;
    }

    void UTextureCube::Serialize(FArchive& Ar)
    {
        UTexture::Serialize(Ar);
        Ar.json["ClassName"] = "UTextureCube";
    }

    void UTextureCube::Deserialize(FArchive& Ar)
    {
        UTexture::Deserialize(Ar);
        UTexture::DeserializeImageData(Ar);
        UpdateResource();
    }

    std::shared_ptr<FImage> UTextureCube::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        std::shared_ptr<FImageCube> image = std::make_shared<FImageCube>(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Channel, 
            ImageInfo.PixelFormat, ImageInfo.NumMips);
        return image;
    }

}