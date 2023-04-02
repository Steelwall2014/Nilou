#include "Texture2DArray.h"

namespace nilou {

    void FTexture2DArrayResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image2DArray)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture2DArrayResource "
                "which requires the type of the image to be IT_Image2DArray, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();
        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();

        FTextureResource::InitRHI();
        auto Texture2DArrayRHI = RHICmdList->RHICreateTexture2DArray(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), Image->GetDepth());
        for (int LayerIndex = 0; LayerIndex < Image->GetNumLayers(); LayerIndex++)
        {
            for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
            {
                RHICmdList->RHIUpdateTexture2DArray(Texture2DArrayRHI.get(), 
                    0, 0, LayerIndex, Image->GetWidth(), Image->GetHeight(), 
                    MipIndex, Image->GetPointer(0, 0, LayerIndex, MipIndex));
            }
        }
        if (NumMips > Image->GetNumMips())
            RHICmdList->RHIGenerateMipmap(Texture2DArrayRHI);
        SamplerRHI.Texture = TextureRHI.get();

        RHIGetError();
    }

    FTextureResource* UTexture2DArray::CreateResource()
    {
        FTexture2DArrayResource* Resource = new FTexture2DArrayResource(Name, TextureParams, NumMips);
        Resource->SetData(ImageData.get());
        return Resource;
    }

    void UTexture2DArray::Serialize(FArchive& Ar)
    {
        UTexture::Serialize(Ar);
        Ar.json["ClassName"] = "UTexture2DArray";
    }

    void UTexture2DArray::Deserialize(FArchive& Ar)
    {
        UTexture::Deserialize(Ar);
        UTexture::DeserializeImageData(Ar);
        UpdateResource();
    }

    std::shared_ptr<FImage> UTexture2DArray::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        std::shared_ptr<FImage2DArray> image = std::make_shared<FImage2DArray>(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Depth, ImageInfo.Channel,
            ImageInfo.PixelFormat, ImageInfo.NumMips);
        return image;
    }

}
