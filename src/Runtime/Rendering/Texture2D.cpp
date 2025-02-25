#include "Texture2D.h"
#include "RenderGraph.h"
#include "RHICommandList.h"
#include "GenerateMips.h"

namespace nilou {

    UTexture2D* UTexture2D::CreateTransient(std::string Name, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat)
    {
        UTexture2D* Texture = nullptr;
        if (InSizeX > 0 && InSizeY > 0)
        {
            Texture = new UTexture2D();
            Texture->Name = Name;
            Texture->ImageData = FImage(InSizeX, InSizeY, InFormat, EImageType::IT_Image2D);
        }
        else
        {
            NILOU_LOG(Error, "Invalid parameters specified for UTexture2D::CreateTransient()");
        }
        return Texture;
    }

    void FTexture2DResource::InitRHI(RenderGraph& Graph)
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

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.TextureType = ETextureDimension::Texture2D;
        Desc.NumMips = NumMips;
        TextureRDG = RenderGraph::CreateExternalTexture(Name, Desc);

        RDGBuffer* StagingBuffer = Graph.CreateBuffer(
            NFormat("Texture \"{}\" InitRHI staging buffer", Name), 
            RDGBufferDesc(Image->GetDataSize()));

        RDGPassDesc PassDesc("FTexture2DResource::InitRHI");
        PassDesc.bNeverCull = true;
        Graph.AddCopyPass(
            PassDesc,
            StagingBuffer,
            TextureRDG,
            [=](RHICommandList& RHICmdList)
            {
                RHIBuffer* StagingBufferRHI = StagingBuffer->GetRHI();
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
                    0);                     // array layer
            });
        
        FGenerateMips::Execute(Graph, TextureRDG, SamplerStateRHI);

        RHIGetError();
    }

    FTextureResource* UTexture2D::CreateResource()
    {
        FTexture2DResource* Resource = new FTexture2DResource(Name, SamplerState, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    void UTexture2D::ReadPixelsRenderThread(RHICommandList& RHICmdList)
    {
        // TODO: Texture layout
        RHIBufferRef StagingBuffer = RHICreateBuffer(0, ImageData.GetDataSize(), EBufferUsageFlags::None, nullptr);
        RHITexture* TextureRHI = GetResource()->TextureRDG->GetRHI();
        RHICmdList.CopyImageToBuffer(TextureRHI, StagingBuffer, 
                    0,                      // mipmap level
                    0, 0, 0,                // x, y, z offset
                    ImageData.GetWidth(),   // width
                    ImageData.GetHeight(),  // height
                    1,                      // depth
                    0);                     // array layer
        // RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
        ImageData.AllocateSpace();
        void* data = RHIMapMemory(StagingBuffer, 0, ImageData.GetDataSize());
            std::memcpy(data, ImageData.GetData(), ImageData.GetDataSize());
        RHIUnmapMemory(StagingBuffer);
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