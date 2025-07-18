#include "Texture3D.h"
#include "RenderGraph.h"
#include "RHICommandList.h"
#include "GenerateMips.h"

namespace nilou {

    void FTexture3DResource::InitRHI(RenderGraph& Graph)
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image3D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture3DResource "
                "which requires the type of the image to be IT_Image3D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        RHIGetError();

        RDGTextureDesc Desc{};
        Desc.Format = Image->GetPixelFormat();
        Desc.SizeX = Image->GetWidth();
        Desc.SizeY = Image->GetHeight();
        Desc.SizeZ = Image->GetDepth();
        Desc.TextureType = ETextureDimension::Texture3D;
        Desc.NumMips = NumMips;
        TextureRDG = RenderGraph::CreatePooledTexture(Name, Desc);

        RDGPassDesc PassDesc{"FTexture3DResource::InitRHI"};
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
                    Image->GetDepth(),      // depth
                    0,                      // base array layer
                    1);                     // num array layers
            });
        
        FGenerateMips::Execute(Graph, TextureRDG.GetReference());

        RHIGetError();
    }

    FTextureResource* UTexture3D::CreateResource()
    {
        FTexture3DResource* Resource = new FTexture3DResource(GetName(), SamplerState, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    // void UTexture3D::Serialize(FArchive& Ar)
    // {
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UTexture3D";
    // }

    // void UTexture3D::Deserialize(FArchive& Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     UTexture::DeserializeImageData(Ar);
    //     UpdateResource();
    // }

    FImage UTexture3D::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, ImageInfo.Depth, 
            ImageInfo.PixelFormat, EImageType::IT_Image3D, ImageInfo.NumMips);
        return image;
    }

}