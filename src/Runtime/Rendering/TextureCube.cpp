#include "TextureCube.h"
#include "RenderGraph.h"
#include "RHICommandList.h"
#include "GenerateMips.h"

namespace nilou {

    void FTextureCubeResource::InitRHI(RenderGraph& Graph)
    {
        if (Image && Image->GetImageType() != EImageType::IT_ImageCube)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureCubeResource "
                "which requires the type of the image to be IT_ImageCube, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.TextureType = ETextureDimension::TextureCube;
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
                    6);                     // array layer
            });
        
        FGenerateMips::Execute(Graph, TextureRDG.get(), SamplerStateRHI.get());

        RHIGetError();
    }

    FTextureResource* UTextureCube::CreateResource()
    {
        FTextureCubeResource* Resource = new FTextureCubeResource(Name, SamplerState, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    // void UTextureCube::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTextureCube";
    // }

    // void UTextureCube::Deserialize(FArchive& Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     UpdateResource();
    // }

    FImage UTextureCube::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, 
            ImageInfo.PixelFormat, EImageType::IT_ImageCube, ImageInfo.NumMips);
        return image;
    }

}