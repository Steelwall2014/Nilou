#include <magic_enum.hpp>
#include <fstream>
#include "Common/Path.h"
#include "Common/Asset/AssetLoader.h"
#include "DynamicRHI.h"
#include "Common/Log.h"

#include "Texture2D.h"
#include "Texture3D.h"
#include "Texture2DArray.h"
#include "TextureCube.h"
#include "VirtualTexture2D.h"

namespace nilou {
		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
    }

    void FTexture::ReleaseRHI()
    {
        TextureRHI = nullptr;
    }

    void FTexture::SetData(FImage* InImage)
    {
        Image = InImage;
    }

    std::shared_ptr<UVirtualTexture> UTexture::MakeVirtualTexture()
    {
        std::shared_ptr<UVirtualTexture> VT = std::make_shared<UVirtualTexture>();
        VT->Name = std::move(Name);
        VT->TextureResource->Name = std::move(this->TextureResource->Name);
        VT->TextureResource->NumMips = std::move(this->TextureResource->NumMips);
        VT->TextureResource->SamplerRHI.Params = std::move(this->TextureResource->SamplerRHI.Params);
        VT->TextureResource->SamplerRHI.ResourceType = std::move(this->TextureResource->SamplerRHI.ResourceType);
        this->TextureResource = nullptr;
        return VT;
    }

    void UTexture::ReadPixelsSync()
    {
        std::mutex m;
        std::unique_lock<std::mutex> lock(m);
        std::condition_variable cv;
        bool pixels_readed = false;
        ENQUEUE_RENDER_COMMAND(UTexture_ReadPixelsSync)(
            [this, &cv, &pixels_readed](FDynamicRHI* RHICmdList)
            {
                ReadPixelsRenderThread(RHICmdList);
                cv.notify_one();
                pixels_readed = true;
            });
        if (!pixels_readed)
            cv.wait(lock, [&pixels_readed] { return pixels_readed == true; });
    }

    void UTexture::PostDeserialize()
    {
        UpdateResource();
    }

    // void UTexture::Serialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.json;
    //     nlohmann::json &content = json["Content"];
    //     content["Name"] = Name;
    //     if (ImageData)
    //     {
    //         nlohmann::json &image = content["ImageData"];
    //         image["Width"] = ImageData->GetWidth();
    //         image["Height"] = ImageData->GetHeight();
    //         image["Channel"] = ImageData->GetChannel();
    //         image["Depth"] = ImageData->GetDepth();
    //         image["DataSize"] = ImageData->GetDataSize();
    //         image["NumMips"] = ImageData->GetNumMips();
    //         image["ImageType"] = magic_enum::enum_name(ImageData->GetImageType());
    //         image["PixelFormat"] = magic_enum::enum_name(ImageData->GetPixelFormat());
    //         if (ImageData->GetData())
    //             Ar.OutBuffers.AddBuffer(image["Data"], ImageData->GetActualDataSize(), ImageData->GetData());
    //     }
    //     content["NumMips"] = NumMips;
    //     content["TextureParams"]["Wrap_S"] = magic_enum::enum_name(TextureParams.Wrap_S);
    //     content["TextureParams"]["Wrap_R"] = magic_enum::enum_name(TextureParams.Wrap_R);
    //     content["TextureParams"]["Wrap_T"] = magic_enum::enum_name(TextureParams.Wrap_T);
    //     content["TextureParams"]["MagFilter"] = magic_enum::enum_name(TextureParams.Mag_Filter);
    //     content["TextureParams"]["MinFilter"] = magic_enum::enum_name(TextureParams.Min_Filter);
    // }

    // void UTexture::Deserialize(FArchive& Ar)
    // {
    //     nlohmann::json &json = Ar.json;

    //     if (json.contains("Content") == false) return;
    //     nlohmann::json& content = json["Content"];

    //     if (content.contains("Name"))
    //         Name = content["Name"];
        
    //     if (content.contains("ImageData"))
    //     {
    //         nlohmann::json& image = content["ImageData"];
    //         ImageCreateInfo ImageInfo;
    //         ImageInfo.Width = image["Width"];
    //         ImageInfo.Height = image["Height"];
    //         ImageInfo.Channel = image["Channel"];
    //         ImageInfo.Depth = image["Depth"];
    //         ImageInfo.NumMips = image["NumMips"];
    //         ImageInfo.ImageType = magic_enum::enum_cast<EImageType>(std::string(image["ImageType"])).value();
    //         ImageInfo.PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
    //         ImageData = CreateImage(ImageInfo);
    //     }

    //     NumMips = content["NumMips"];
    //     TextureParams.Wrap_R = magic_enum::enum_cast<ETextureWrapModes>(std::string(content["TextureParams"]["Wrap_R"])).value();
    //     TextureParams.Wrap_S = magic_enum::enum_cast<ETextureWrapModes>(std::string(content["TextureParams"]["Wrap_S"])).value();
    //     TextureParams.Wrap_T = magic_enum::enum_cast<ETextureWrapModes>(std::string(content["TextureParams"]["Wrap_T"])).value();
    //     TextureParams.Mag_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(content["TextureParams"]["MagFilter"])).value();
    //     TextureParams.Min_Filter = magic_enum::enum_cast<ETextureFilters>(std::string(content["TextureParams"]["MinFilter"])).value();

    // }

    // void UTexture::DeserializeImageData(FArchive& Ar)
    // {
    //     nlohmann::json& json = Ar.json;
    //     nlohmann::json& content = json["Content"];
    //     nlohmann::json& image = content["ImageData"];
    //     uint32 BufferOffset = image["Data"]["BufferOffset"];
    //     uint64 DataSize = image["DataSize"];
    //     ImageData->AllocateSpace();
    //     std::copy(
    //         Ar.InBuffer.get()+BufferOffset, 
    //         Ar.InBuffer.get()+glm::min(DataSize, ImageData->GetDataSize()), 
    //         ImageData->GetData());
    // }

}