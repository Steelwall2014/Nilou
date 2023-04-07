#include "TextureRenderTarget.h"

namespace nilou {

    void FTextureRenderTarget2DResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureRenderTarget2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }
        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();

        FTextureRenderTargetResource::InitRHI();
        auto Texture2DRHI = RHICmdList->RHICreateTexture2D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight());
        TextureRHI = Texture2DRHI;
        for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
        {
            RHICmdList->RHIUpdateTexture2D(Texture2DRHI.get(), 
                0, 0, Image->GetWidth(), Image->GetHeight(), 
                MipIndex, Image->GetPointer(0, 0, 0, MipIndex));
        }
        if (NumMips > Image->GetNumMips())
            RHICmdList->RHIGenerateMipmap(Texture2DRHI);
        RenderTargetFramebuffer = RHICmdList->RHICreateFramebuffer();
        RenderTargetFramebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, Texture2DRHI);
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTextureRenderTargetCubeResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_ImageCube)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureRenderTargetCubeResource "
                "which requires the type of the image to be IT_ImageCube, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();

        FTextureRenderTargetResource::InitRHI();
        void* data_pointers[6];
        auto CubeImage = static_cast<FImageCube*>(Image);
        for (int i = 0; i < 6; i++)
            data_pointers[i] = CubeImage->GetPointer(0, 0, i);

        FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
        auto TextureCubeRHI = RHICmdList->RHICreateTextureCube(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight());
        TextureRHI = TextureCubeRHI;
        SamplerRHI.Texture = TextureRHI.get();
        
        for (int i = 0; i < 6; i++)
        {
            for (int MipIndex = 0; MipIndex < Image->GetNumMips(); MipIndex++)
            {
                RHICmdList->RHIUpdateTextureCube(TextureCubeRHI.get(), 
                    0, 0, i, 
                    Image->GetWidth(), Image->GetHeight(), MipIndex, 
                    Image->GetPointer(0, 0, i, MipIndex));
            }

            RenderTargetTextureViews[i] = RHICmdList->RHICreateTextureView2D(
                TextureRHI.get(), TextureRHI->GetFormat(), 
                0, 1, i);
        
            RenderTargetFramebuffers[i] = RHICmdList->RHICreateFramebuffer();
            RenderTargetFramebuffers[i]->AddAttachment(
                EFramebufferAttachment::FA_Color_Attachment0, 
                RenderTargetTextureViews[i]);
        }

        RHIGetError();
    
    }

    FTextureRenderTargetResource* UTextureRenderTarget::GetRenderTargetResource()
    {
        if (GetResource() && GetResource()->IsInitialized())
        {
            return static_cast<FTextureRenderTargetResource*>(GetResource());
        }
        return nullptr;
    }

    void UTextureRenderTarget::Serialize(FArchive& Ar)
    {
        UTexture::Serialize(Ar);
        Ar.json["Content"]["ClearColor"].push_back(ClearColor.x);
        Ar.json["Content"]["ClearColor"].push_back(ClearColor.y);
        Ar.json["Content"]["ClearColor"].push_back(ClearColor.z);
    }

    void UTextureRenderTarget::Deserialize(FArchive& Ar)
    {
        UTexture::Deserialize(Ar);
        UTexture::DeserializeImageData(Ar);
        ClearColor.x = Ar.json["Content"]["ClearColor"][0].get<double>();
        ClearColor.y = Ar.json["Content"]["ClearColor"][1].get<double>();
        ClearColor.z = Ar.json["Content"]["ClearColor"][2].get<double>();
    }

    FTextureResource* UTextureRenderTarget2D::CreateResource()
    {
        FTextureRenderTarget2DResource* Resource = new FTextureRenderTarget2DResource(Name, TextureParams, NumMips);
        Resource->SetData(ImageData.get());
        Resource->ClearColor = ClearColor;
        return Resource;
    }

    std::shared_ptr<FImage> UTextureRenderTarget2D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        std::shared_ptr<FImage2D> image = std::make_shared<FImage2D>(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Channel, 
            ImageInfo.PixelFormat, ImageInfo.NumMips);
        return image;
    }

    void UTextureRenderTarget2D::Serialize(FArchive& Ar)
    {
        UTextureRenderTarget::Serialize(Ar);
        Ar.json["ClassName"] = "UTextureRenderTarget2D";
    }

    void UTextureRenderTarget2D::Deserialize(FArchive& Ar)
    {
        UTextureRenderTarget::Deserialize(Ar);
        UpdateResource();
    }

    FTextureResource* UTextureRenderTargetCube::CreateResource()
    {
        FTextureRenderTargetCubeResource* Resource = new FTextureRenderTargetCubeResource(Name, TextureParams, NumMips);
        Resource->SetData(ImageData.get());
        Resource->ClearColor = ClearColor;
        return Resource;
    }

    std::shared_ptr<FImage> UTextureRenderTargetCube::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        std::shared_ptr<FImageCube> image = std::make_shared<FImageCube>(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Channel, 
            ImageInfo.PixelFormat, ImageInfo.NumMips);
        return image;
    }

    void UTextureRenderTargetCube::Serialize(FArchive& Ar)
    {
        UTextureRenderTarget::Serialize(Ar);
        Ar.json["ClassName"] = "UTextureRenderTargetCube";
    }

    void UTextureRenderTargetCube::Deserialize(FArchive& Ar)
    {
        UTextureRenderTarget::Deserialize(Ar);
        UpdateResource();
    }

}