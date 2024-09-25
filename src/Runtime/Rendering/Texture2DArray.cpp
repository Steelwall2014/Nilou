#include "Texture2DArray.h"
#include "RenderGraph.h"
#include "RHICommandList.h"
#include "GenerateMips.h"

namespace nilou {

    void FTexture2DArrayResource::InitRHI(RenderGraph& Graph)
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

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.ArraySize = Image->GetNumLayers();
        Desc.TextureType = ETextureDimension::Texture2DArray;
        Desc.NumMips = NumMips;
        TextureRDG = RenderGraph::CreatePersistentTexture(Name, Desc);

        RDGBuffer* StagingBuffer = Graph.CreateBuffer(
            fmt::format("Texture \"{}\" InitRHI staging buffer", Name), 
            RDGBufferDesc(Image->GetDataSize()));

        RDGCopyPassDesc PassDesc{};
        PassDesc.Source = StagingBuffer;
        PassDesc.Destination = TextureRDG.get();
        PassDesc.bNeverCull = true;
        Graph.AddCopyPass(
            PassDesc,
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
                    Image->GetNumLayers()); // array layer
            });
        
        FGenerateMips::Execute(Graph, TextureRDG.get(), SamplerStateRHI.get());

        RHIGetError();
    }

    FTextureResource* UTexture2DArray::CreateResource()
    {
        FTexture2DArrayResource* Resource = new FTexture2DArrayResource(Name, SamplerState, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    // void UTexture2DArray::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTexture2DArray";
    // }

    // void UTexture2DArray::Deserialize(FArchive& Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     UpdateResource();
    // }

    FImage UTexture2DArray::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Depth, 
            ImageInfo.PixelFormat, EImageType::IT_Image2DArray, ImageInfo.NumMips);
        return image;
    }

}
