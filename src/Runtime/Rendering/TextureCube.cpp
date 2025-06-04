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
        TextureRDG = RenderGraph::CreateExternalTexture(Name, Desc);

        RDGPassDesc PassDesc{"FTextureCubeResource::InitRHI"};
        PassDesc.bNeverCull = true;
        Graph.AddCopyPass(
            PassDesc,
            nullptr,
            TextureRDG,
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
                    6);                     // array layer
            });
        
        FGenerateMips::Execute(Graph, TextureRDG);

        RHIGetError();
    }

    FTextureResource* UTextureCube::CreateResource()
    {
        FTextureCubeResource* Resource = new FTextureCubeResource(GetName(), SamplerState, NumMips);
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