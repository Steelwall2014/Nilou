#include "Texture2D.h"

namespace nilou {

    void FTexture2DResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();

        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
        FTextureResource::InitRHI();
        auto Texture2DRHI = RHICmdList->RHICreateTexture2D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), TexCreate_CPUWritable);
        for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
        {
            RHICmdList->RHIUpdateTexture2D(Texture2DRHI.get(), 
                0, 0, Image->GetWidth(), Image->GetHeight(), 
                MipIndex, Image->GetPointer(0, 0, 0, MipIndex));
        }
        if (NumMips > Image->GetNumMips())
            RHICmdList->RHIGenerateMipmap(Texture2DRHI);

        TextureRHI = Texture2DRHI;
        SamplerRHI.Texture = TextureRHI.get();
    }

    FTextureResource* UTexture2D::CreateResource()
    {
        FTexture2DResource* Resource = new FTexture2DResource(Name, TextureParams, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    void UTexture2D::ReadPixelsRenderThread(FDynamicRHI* RHICmdList)
    {
        uint8* data = RHICmdList->RHIReadImagePixel(std::static_pointer_cast<RHITexture2D>(GetResource()->TextureRHI));
        ImageData.AllocateSpace();
        std::copy(data, data+ImageData.GetDataSize(), ImageData.GetData());
        delete[] data;
    }

    // void UTexture2D::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTexture2D";
    // }

    // void UTexture2D::Deserialize(FArchive& Ar)
    // {        
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     UpdateResource();
    // }

    FImage UTexture2D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, 
            ImageInfo.PixelFormat, EImageType::IT_Image2D, ImageInfo.NumMips);
        return image;
    }
}