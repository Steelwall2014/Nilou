#include <magic_enum.hpp>
#include "Texture.h"
#include "Common/AssetLoader.h"
#include "DynamicRHI.h"

namespace nilou {

		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D("", Image->PixelFormat, NumMips, Image->Width, Image->Height, Image->data);
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTexture::ReleaseRHI()
    {
        TextureRHI = nullptr;
    }


    void UTexture::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UTexture";
        nlohmann::json content = nlohmann::json();
        content["Name"] = Name;
        if (TextureResource != nullptr)
        {
            nlohmann::json texture_resource = nlohmann::json();
            texture_resource["NumMips"] = TextureResource->NumMips;
            texture_resource["Wrap_S"] = magic_enum::enum_name(GetWrapS());
            texture_resource["Wrap_R"] = magic_enum::enum_name(GetWrapR());
            texture_resource["Wrap_T"] = magic_enum::enum_name(GetWrapT());
            texture_resource["MagFilter"] = magic_enum::enum_name(GetMagFilter());
            texture_resource["MinFilter"] = magic_enum::enum_name(GetMinFilter());
            // texture_resource["TextureType"] = magic_enum::enum_name(GetTextureType());
            nlohmann::json image = nlohmann::json();
            image["Width"] = TextureResource->Image->Width;
            image["Height"] = TextureResource->Image->Height;
            image["Channel"] = TextureResource->Image->Channel;
            image["DataSize"] = TextureResource->Image->data_size;
            image["PixelFormat"] = magic_enum::enum_name(TextureResource->Image->PixelFormat);
            image["Data"] = SerializeHelper::Base64Encode(TextureResource->Image->data, TextureResource->Image->data_size);
            texture_resource["Image"] = image;
            content["TextureResource"] = texture_resource;
        }
        json["Content"] = content;
    }

    void UTexture::Deserialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        if (json["ClassName"] != "UTexture") return;

        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];

        if (content.contains("Name") == false) return;
        Name = content["Name"];
        
        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        if (texture_resource.contains("Image") == false) return;
        nlohmann::json image = texture_resource["Image"];
        std::shared_ptr<FImage> Image = std::make_shared<FImage>();
        Image->Width = image["Width"];
        Image->Height = image["Height"];
        Image->Channel = image["Channel"];
        Image->data_size = image["DataSize"];
        Image->PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
        std::string data = SerializeHelper::Base64Decode(image["Data"].get<std::string>());
        Image->data = new unsigned char[data.size()];
        std::memcpy(Image->data, data.data(), data.size());
        
        int NumMips = 1;
        if (texture_resource.contains("NumMips"))
            NumMips = texture_resource["NumMips"];
        TextureResource->Image = Image;
        TextureResource->NumMips = NumMips;

        RHITextureParams &Params = TextureResource->SamplerRHI.Params;

        if (texture_resource.contains("Wrap_R"))
            Params.Wrap_R = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_R"])).value();
            
        if (texture_resource.contains("Wrap_S"))
            Params.Wrap_S = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_S"])).value();
            
        if (texture_resource.contains("Wrap_T"))
            Params.Wrap_T = magic_enum::enum_cast<ETextureWrapModes>(std::string(texture_resource["Wrap_T"])).value();
            
        if (texture_resource.contains("MagFilter"))
            Params.Mag_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(texture_resource["MagFilter"])).value();
            
        if (texture_resource.contains("MinFilter"))
            Params.Min_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(texture_resource["MinFilter"])).value();
            
        // if (texture_resource.contains("TextureType"))
            // Params.TextureType = magic_enum::enum_cast<ETextureType>(std::string(texture_resource["TextureType"])).value();

        BeginInitResource(TextureResource.get());
    }

}