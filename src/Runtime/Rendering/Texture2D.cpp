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
            Image->GetWidth(), Image->GetHeight());
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
        Resource->SetData(ImageData.get());
        return Resource;
    }

    void UTexture2D::Serialize(FArchive& Ar)
    {
        UTexture::Serialize(Ar);
        Ar.json["ClassName"] = "UTexture2D";
    }

    void UTexture2D::Deserialize(FArchive& Ar)
    {        
        UTexture::Deserialize(Ar);
        UTexture::DeserializeImageData(Ar);
        UpdateResource();
        // nlohmann::json &json = Ar.json;

        // if (json.contains("Content") == false) return;
        // nlohmann::json& content = json["Content"];

        // if (content.contains("Name"))
        //     Name = content["Name"];
        
        // if (content.contains("TextureResource") == false) return;
        // nlohmann::json& texture_resource = content["TextureResource"];

        // if (texture_resource.contains("Image") == false) return;

        // nlohmann::json image = texture_resource["Image"];
        // EImageType ImageType = EImageType::IT_Image2D;
        // if (image.contains("ImageType"))
        //     ImageType = magic_enum::enum_cast<EImageType>(std::string(image["ImageType"])).value();
        
        // NumMips = 1;
        // if (texture_resource.contains("NumMips"))
        //     NumMips = texture_resource["NumMips"];
        // uint32 Width, Height, Channel, Depth = 1, ImageMipmap = 1;
        // EPixelFormat PixelFormat;
        // Width = image["Width"];
        // Height = image["Height"];
        // Channel = image["Channel"];
        // if (image.contains("Depth"))
        //     Depth = image["Depth"];
        // PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
        
        // switch (ImageType)
        // {
        // case EImageType::IT_Image2D:
        //     ImageData = std::make_shared<FImage2D>(Width, Height, Channel, PixelFormat, ImageMipmap);
        //     TextureResource = new FTexture2DResource(Name, NumMips);
        //     break;
        // };

        // RHITextureParams &Params = TextureResource->SamplerRHI.Params;

        // if (texture_resource.contains("Wrap_R"))
        //     Params.Wrap_R = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_R"])).value();
            
        // if (texture_resource.contains("Wrap_S"))
        //     Params.Wrap_S = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_S"])).value();
            
        // if (texture_resource.contains("Wrap_T"))
        //     Params.Wrap_T = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_T"])).value();
            
        // if (texture_resource.contains("MagFilter"))
        //     Params.Mag_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(texture_resource["MagFilter"])).value();
            
        // if (texture_resource.contains("MinFilter"))
        //     Params.Min_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(texture_resource["MinFilter"])).value();
            
        // if (texture_resource.contains("TextureType"))
        //     TextureResource->TextureType = magic_enum::enum_cast<ETextureType>(std::string(texture_resource["TextureType"])).value();

        // uint32 BufferOffset = image["Data"]["BufferOffset"];
        // uint64 DataSize = image["DataSize"];
        // ImageData->AllocateSpace();
        // std::copy(
        //     Ar.InBuffer.get()+BufferOffset, 
        //     Ar.InBuffer.get()+glm::min(DataSize, ImageData->GetDataSize()), 
        //     ImageData->GetData());

        // UpdateResource();
    }

    std::shared_ptr<FImage> UTexture2D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        std::shared_ptr<FImage2D> image = std::make_shared<FImage2D>(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Channel, 
            ImageInfo.PixelFormat, ImageInfo.NumMips);
        return image;
    }
}