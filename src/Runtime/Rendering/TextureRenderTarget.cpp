#include "TextureRenderTarget.h"
#include "RenderGraph.h"
#include "RHICommandList.h"

namespace nilou {

    void FTextureRenderTarget2DResource::InitRHI(RenderGraph& Graph)
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureRenderTarget2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.NumMips = NumMips;
        TextureRDG = RenderGraph::CreatePooledTexture(Name, Desc);

        RDGPassDesc PassDesc{"FTextureRenderTarget2DResource::InitRHI"};
        PassDesc.bNeverCull = true;
        Graph.AddCopyPass(
            PassDesc,
            nullptr,
            TextureRDG.GetReference(),
            [=, this](RHICommandList& RHICmdList)
            {
                RHIBuffer* StagingBufferRHI = RHICmdList.AcquireStagingBuffer(Image->GetDataSize());
                void* Data = RHIMapMemory(StagingBufferRHI, 0, Image->GetDataSize());
                    std::memcpy(Data, Image->GetPointer(0, 0, 0), Image->GetAllocatedDataSize());
                RHIUnmapMemory(StagingBufferRHI);

                RHITexture* TextureRHI = TextureRDG->GetRHI();
                RHICmdList.CopyBufferToImage(StagingBufferRHI, TextureRHI, 
                    0,                      // mipmap level
                    0, 0, 0,                // x, y, z offset
                    Image->GetWidth(),      // width
                    Image->GetHeight(),     // height
                    1,                      // depth
                    0,                      // base array layer
                    1);                     // num array layers
            });

        RHIGetError();
    }

    void FTextureRenderTargetCubeResource::InitRHI(RenderGraph& Graph)
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

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.ArraySize = 6;
        Desc.TextureType = ETextureDimension::TextureCube;
        Desc.NumMips = NumMips;
        TextureRDG = RenderGraph::CreatePooledTexture(Name, Desc);

        // RDGPassDesc PassDesc{"FTextureRenderTargetCubeResource::InitRHI"};
        // PassDesc.bNeverCull = true;
        // Graph.AddCopyPass(
        //     PassDesc,
        //     nullptr,
        //     TextureRDG,
        //     [=, this](RHICommandList& RHICmdList)
        //     {
        //         RHIBuffer* StagingBufferRHI = RHICmdList.AcquireStagingBuffer(Image->GetDataSize());
        //         void* Data = RHIMapMemory(StagingBufferRHI, 0, Image->GetDataSize());
        //             std::memcpy(Data, Image->GetPointer(0, 0, 0), Image->GetAllocatedDataSize());
        //         RHIUnmapMemory(StagingBufferRHI);

        //         RHITexture* TextureRHI = TextureRDG->GetRHI();
        //         RHICmdList.CopyBufferToImage(StagingBufferRHI, TextureRHI, 
        //             0,                      // mipmap level
        //             0, 0, 0,                // x, y, z offset
        //             Image->GetWidth(),      // width
        //             Image->GetHeight(),     // height
        //             1,                      // depth
        //             0,                      // base array layer     
        //             6);                     // num array layers
        //     });

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

    // void UTextureRenderTarget::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["Content"]["ClearColor"].push_back(ClearColor.x);
    //     Ar.json["Content"]["ClearColor"].push_back(ClearColor.y);
    //     Ar.json["Content"]["ClearColor"].push_back(ClearColor.z);
    // }

    // void UTextureRenderTarget::Deserialize(FArchive& Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     ClearColor.x = Ar.json["Content"]["ClearColor"][0].get<double>();
    //     ClearColor.y = Ar.json["Content"]["ClearColor"][1].get<double>();
    //     ClearColor.z = Ar.json["Content"]["ClearColor"][2].get<double>();
    // }

    FTextureResource* UTextureRenderTarget2D::CreateResource()
    {
        FTextureRenderTarget2DResource* Resource = new FTextureRenderTarget2DResource(GetName(), SamplerState, NumMips);
        Resource->SetData(&ImageData);
        Resource->ClearColor = ClearColor;
        return Resource;
    }

    FImage UTextureRenderTarget2D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, 
            ImageInfo.PixelFormat, EImageType::IT_Image2D, ImageInfo.NumMips);
        return image;
    }

    // void UTextureRenderTarget2D::Serialize(FArchive& Ar)
    // {
    //     UTextureRenderTarget::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTextureRenderTarget2D";
    // }

    // void UTextureRenderTarget2D::Deserialize(FArchive& Ar)
    // {
    //     UTextureRenderTarget::Deserialize(Ar);
    //     UpdateResource();
    // }

    FTextureResource* UTextureRenderTargetCube::CreateResource()
    {
        FTextureRenderTargetCubeResource* Resource = new FTextureRenderTargetCubeResource(GetName(), SamplerState, NumMips);
        Resource->SetData(&ImageData);
        Resource->ClearColor = ClearColor;
        return Resource;
    }

    FImage UTextureRenderTargetCube::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, 
            ImageInfo.PixelFormat, EImageType::IT_ImageCube, ImageInfo.NumMips);
        return image;
    }

    // void UTextureRenderTargetCube::Serialize(FArchive& Ar)
    // {
    //     UTextureRenderTarget::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTextureRenderTargetCube";
    // }

    // void UTextureRenderTargetCube::Deserialize(FArchive& Ar)
    // {
    //     UTextureRenderTarget::Deserialize(Ar);
    //     UpdateResource();
    // }

}