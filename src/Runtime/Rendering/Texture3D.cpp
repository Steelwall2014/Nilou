#include "Texture3D.h"

namespace nilou {

    void FTexture3DResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image3D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture3DResource "
                "which requires the type of the image to be IT_Image3D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();
        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();

        FTextureResource::InitRHI();
        auto Texture3DRHI = RHICmdList->RHICreateTexture3D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), Image->GetDepth());
        TextureRHI = Texture3DRHI;
        
        for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
        {
            RHICmdList->RHIUpdateTexture3D(Texture3DRHI.get(), 
                0, 0, 0, Image->GetWidth(), Image->GetHeight(), Image->GetDepth(), 
                MipIndex, Image->GetPointer(0, 0, 0, MipIndex));
        }
        SamplerRHI.Texture = TextureRHI.get();

        RHIGetError();
    }

    FTextureResource* UTexture3D::CreateResource()
    {
        FTexture3DResource* Resource = new FTexture3DResource(Name, TextureParams, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    // void UTexture3D::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTexture3D";
    // }

    // void UTexture3D::Deserialize(FArchive& Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     UpdateResource();
    // }

    FImage UTexture3D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Depth, 
            ImageInfo.PixelFormat, EImageType::IT_Image3D, ImageInfo.NumMips);
        return image;
    }

}