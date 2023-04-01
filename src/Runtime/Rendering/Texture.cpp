#include <magic_enum.hpp>
#include <fstream>
#include "Common/Path.h"
#include "Texture.h"
#include "Common/Asset/AssetLoader.h"
#include "DynamicRHI.h"
#include "RenderingThread.h"
#include "Common/Log.h"

namespace nilou {
		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
    }

    void FTexture::ReleaseRHI()
    {
        TextureRHI = nullptr;
    }

    void FTexture::SetData(std::weak_ptr<FImage> Image)
    {
        WeakImage = Image;
        if (WeakImage.expired())
            return;
        ENQUEUE_RENDER_COMMAND(FTexture_SetData)(
            [this](FDynamicRHI*)
            {
                InitRHI();
            });
    }

    void FTexture2DResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), 
            Image->GetData());
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTexture3DResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_Image3D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture3DResource "
                "which requires the type of the image to be IT_Image3D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture3D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), Image->GetDepth(),
            Image->GetData());
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTexture2DArrayResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_Image2DArray)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTexture2DArrayResource "
                "which requires the type of the image to be IT_Image2DArray, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2DArray(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), Image->GetDepth(), 
            Image->GetData());
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTextureCubeResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_ImageCube)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureCubeResource "
                "which requires the type of the image to be IT_ImageCube, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }
        void* data_pointers[6];
        auto CubeImage = static_cast<FImageCube*>(Image.get());
        for (int i = 0; i < 6; i++)
            data_pointers[i] = CubeImage->GetPointer(0, 0, i);

        FTextureResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTextureCube(
            Name, Image->GetPixelFormat(), 
            NumMips, Image->GetWidth(), Image->GetHeight(), 
            data_pointers);
        SamplerRHI.Texture = TextureRHI.get();
    }
		
    void FVirtualTexture2DResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FVirtualTexture2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateSparseTexture2D(
            Name, Image->GetPixelFormat(), 
            NumMips, Image->GetWidth(), Image->GetHeight());
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTextureRenderTarget2DResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_Image2D)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureRenderTarget2DResource "
                "which requires the type of the image to be IT_Image2D, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureRenderTargetResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            Name, Image->GetPixelFormat(), NumMips, 
            Image->GetWidth(), Image->GetHeight(), 
            Image->GetData());
        DepthStencilRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(
            Name, EPixelFormat::PF_D24S8, 1, 
            Image->GetWidth(), Image->GetHeight(), 
            nullptr);
        auto Texture2DRHI = std::static_pointer_cast<RHITexture2D>(TextureRHI);
        Framebuffer = FDynamicRHI::GetDynamicRHI()->RHICreateFramebuffer();
        Framebuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, Texture2DRHI);
        Framebuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, DepthStencilRHI);
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTextureRenderTargetCubeResource::InitRHI()
    {
        if (WeakImage.expired())
            return;
        auto Image = WeakImage.lock();
        if (Image->GetImageType() != EImageType::IT_ImageCube)
        {
            NILOU_LOG(Error, 
                "This texture resource is FTextureRenderTargetCubeResource "
                "which requires the type of the image to be IT_ImageCube, but given {}", 
                magic_enum::enum_name(Image->GetImageType()));
            return;
        }

        FTextureRenderTargetResource::InitRHI();
        void* data_pointers[6];
        auto CubeImage = static_cast<FImageCube*>(Image.get());
        for (int i = 0; i < 6; i++)
            data_pointers[i] = CubeImage->GetPointer(0, 0, i);

        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTextureCube(
            Name, Image->GetPixelFormat(), 
            NumMips, Image->GetWidth(), Image->GetHeight(), 
            data_pointers);
        SamplerRHI.Texture = TextureRHI.get();
    }

    std::shared_ptr<UVirtualTexture> UTexture::MakeVirtualTexture()
    {
        std::shared_ptr<UVirtualTexture> VT = std::make_shared<UVirtualTexture>();
        VT->Name = std::move(Name);
        VT->Image = std::move(this->Image);
        VT->TextureResource->Name = std::move(this->TextureResource->Name);
        VT->TextureResource->NumMips = std::move(this->TextureResource->NumMips);
        VT->TextureResource->SamplerRHI.Params = std::move(this->TextureResource->SamplerRHI.Params);
        VT->TextureResource->SamplerRHI.ResourceType = std::move(this->TextureResource->SamplerRHI.ResourceType);
        VT->TextureResource->SetData(VT->Image);
        this->TextureResource = nullptr;
        return VT;
    }

    void UTexture::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UTexture";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        content["TextureType"] = GetTextureType();
        if (TextureResource != nullptr)
        {
            nlohmann::json &texture_resource = content["TextureResource"];
            texture_resource["NumMips"] = TextureResource->NumMips;
            texture_resource["Wrap_S"] = magic_enum::enum_name(GetTextureParams().Wrap_S);
            texture_resource["Wrap_R"] = magic_enum::enum_name(GetTextureParams().Wrap_R);
            texture_resource["Wrap_T"] = magic_enum::enum_name(GetTextureParams().Wrap_T);
            texture_resource["MagFilter"] = magic_enum::enum_name(GetTextureParams().Mag_Filter);
            texture_resource["MinFilter"] = magic_enum::enum_name(GetTextureParams().Min_Filter);
            // texture_resource["TextureType"] = magic_enum::enum_name(GetTextureType());
            nlohmann::json &image = texture_resource["Image"];
            image["Width"] = Image->GetWidth();
            image["Height"] = Image->GetHeight();
            image["Channel"] = Image->GetChannel();
            image["DataSize"] = Image->GetDataSize();
            image["PixelFormat"] = magic_enum::enum_name(Image->GetPixelFormat());
            if (Image->GetData())
                Ar.OutBuffers.AddBuffer(image["Data"], Image->GetActualDataSize(), Image->GetData());
        }
    }

    void UTexture::DeserializeWithoutImageData(FArchive& Ar)
    {
        nlohmann::json &json = Ar.json;
        if (json["ClassName"] != "UTexture" && json["ClassName"] != "UVirtualTexture") return;

        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];

        if (content.contains("Name") == false) return;
        Name = content["Name"];
        
        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        if (texture_resource.contains("Image") == false) return;
        int NumMips = 1;
        if (texture_resource.contains("NumMips"))
            NumMips = texture_resource["NumMips"];

        nlohmann::json image = texture_resource["Image"];
        EImageType ImageType = EImageType::IT_Image2D;
        if (image.contains("ImageType"))
            ImageType = magic_enum::enum_cast<EImageType>(std::string(image["ImageType"])).value();
        uint32 Width, Height, Channel, Depth = 1, ImageMipmap = NumMips;
        EPixelFormat PixelFormat;
        Width = image["Width"];
        Height = image["Height"];
        Channel = image["Channel"];
        if (image.contains("Mipmap"))
            ImageMipmap = image["Mipmap"];
        if (image.contains("Depth"))
            Depth = image["Depth"];
        PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
        
        switch (ImageType)
        {
        case EImageType::IT_Image2D:
            Image = std::make_shared<FImage2D>(Width, Height, Channel, PixelFormat, ImageMipmap);
            TextureResource = std::make_unique<FTexture2DResource>(Name, NumMips);
            break;
        case EImageType::IT_Image3D:
            Image = std::make_shared<FImage3D>(Width, Height, Channel, Depth, PixelFormat, ImageMipmap);
            TextureResource = std::make_unique<FTexture3DResource>(Name, NumMips);
            break;
        case EImageType::IT_Image2DArray:
            Image = std::make_shared<FImage2DArray>(Width, Height, Channel, Depth, PixelFormat, ImageMipmap);
            TextureResource = std::make_unique<FTexture2DArrayResource>(Name, NumMips);
            break;
        case EImageType::IT_ImageCube:
            Image = std::make_shared<FImageCube>(Width, Height, Channel, PixelFormat, ImageMipmap);
            TextureResource = std::make_unique<FTextureCubeResource>(Name, NumMips);
            break;
        };

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
            
        if (texture_resource.contains("TextureType"))
            TextureResource->TextureType = magic_enum::enum_cast<ETextureType>(std::string(texture_resource["TextureType"])).value();
    }

    void UTexture::Deserialize(FArchive &Ar)
    {
        DeserializeWithoutImageData(Ar);
        nlohmann::json &json = Ar.json;
        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];

        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        nlohmann::json image = texture_resource["Image"];
        uint32 BufferOffset = image["Data"]["BufferOffset"];
        uint64 DataSize = image["DataSize"];
        Image->AllocateSpace();
        std::copy(
            Ar.InBuffer.get()+BufferOffset, 
            Ar.InBuffer.get()+glm::min(DataSize, Image->GetDataSize()), 
            Image->GetData());

        TextureResource->SetData(Image);
    }

    uint32 TileToAddress()
    {

    }

    UVirtualTexture::UVirtualTexture()
        : LruCache(0)
    {

    }

    void UVirtualTexture::UpdateBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel)
    {
        uint32 NumTileX = this->NumTileX >> MipmapLevel;
        uint32 NumTileY = this->NumTileY >> MipmapLevel;
        uvec2 tile_min = UV_Min * vec2(NumTileX, NumTileY);
        uvec2 tile_max = UV_Max * vec2(NumTileX, NumTileY);
        tile_min = glm::clamp(tile_min, uvec2(0), uvec2(NumTileX-1, NumTileY-1));
        tile_max = glm::clamp(tile_max, uvec2(0), uvec2(NumTileX-1, NumTileY-1));
        if (tile_min.x == tile_max.x || tile_min.y == tile_max.y)
        {
            for (int i = tile_min.x; i <= tile_max.x; i++)
            {
                for (int j = tile_min.y; j <= tile_max.y; j++)
                {
                    UpdateTile(i, j, MipmapLevel);
                }
            }
        }
        else 
        {
            for (int i = tile_min.x; i < tile_max.x; i++)
            {
                for (int j = tile_min.y; j < tile_max.y; j++)
                {
                    UpdateTile(i, j, MipmapLevel);
                }
            }
        }
    }

    void UVirtualTexture::UpdateBoundSync(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel)
    {
        UV_Min = glm::clamp(UV_Min, vec2(0), vec2(1));
        UV_Max = glm::clamp(UV_Max, vec2(0), vec2(1));
        uint32 NumTileX = this->NumTileX >> MipmapLevel;
        uint32 NumTileY = this->NumTileY >> MipmapLevel;
        uvec2 tile_min = UV_Min * vec2(NumTileX, NumTileY);
        uvec2 tile_max = UV_Max * vec2(NumTileX, NumTileY);
        for (int i = tile_min.x; i < tile_max.x; i++)
        {
            for (int j = tile_min.y; j < tile_max.y; j++)
            {
                UpdateTileSync(i, j, MipmapLevel);
            }
        }
    }

    void UVirtualTexture::UnloadBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel)
    {
        UV_Min = glm::clamp(UV_Min, vec2(0), vec2(1));
        UV_Max = glm::clamp(UV_Max, vec2(0), vec2(1));
        uint32 NumTileX = this->NumTileX >> MipmapLevel;
        uint32 NumTileY = this->NumTileY >> MipmapLevel;
        uvec2 tile_min = UV_Min * vec2(NumTileX, NumTileY);
        uvec2 tile_max = UV_Max * vec2(NumTileX, NumTileY);
        for (int i = tile_min.x; i < tile_max.x; i++)
        {
            for (int j = tile_min.y; j < tile_max.y; j++)
            {
                UnloadTile(i, j, MipmapLevel);
            }
        }
    }

    void UVirtualTexture::UnloadTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel)
    {
        ENQUEUE_RENDER_COMMAND(UVirtualTexture_UnloadTile)(
            [this, TileX, TileY, MipmapLevel](FDynamicRHI* RHICmdList)
            {
                auto &Tile = Tiles[MipmapLevel][TileX][TileY];
                std::unique_lock<std::mutex> lock(Tile->mutex);
                if (TextureResource == nullptr)
                    return;
                if (Tile->bCommited && TextureResource && TextureResource->TextureRHI)
                {
                    RHICmdList->RHISparseTextureUnloadTile(
                        TextureResource->TextureRHI.get(), TileX, TileY, MipmapLevel);
                    Tile->bCommited = false;
                }
            });
    }

    void UVirtualTexture::UpdateTile(uint32 InTileX, uint32 InTileY, uint32 MipmapLevel)
    {
        thread_pool.push_task([this, InTileX, InTileY, MipmapLevel]() {
            UpdateTileInternal(InTileX, InTileY, MipmapLevel);
        });
    }

    void UVirtualTexture::UpdateTileSync(uint32 InTileX, uint32 InTileY, uint32 MipmapLevel)
    {
        UpdateTileInternal(InTileX, InTileY, MipmapLevel);
    }

    void UVirtualTexture::UpdateTileInternal(uint32 TileX, uint32 TileY, uint32 MipmapLevel)
    {
        MipmapLevel = glm::clamp(MipmapLevel, 0u, uint32(TextureResource->NumMips));
        auto Tile = Tiles[MipmapLevel][TileX][TileY].get();
        std::unique_lock<std::mutex> lock(Tile->mutex, std::try_to_lock);
        if (!lock.owns_lock())
            return;

        auto ExpelledTile = LruCache.Put_ThreadSafe(Tile, Tile);
        if (ExpelledTile.has_value())
        {
            VirtualTextureTile* Tile = ExpelledTile.value().second;
            UnloadTile(Tile->TileX, Tile->TileY, Tile->MipmapLevel);
        }

        if (Tile->bCommited || TextureResource == nullptr)
            return;  
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(Image->GetPixelFormat());
        ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(GetTextureType(), Image->GetPixelFormat());

        std::shared_ptr<char[]> data = std::make_shared<char[]>(PageSize.x*PageSize.y*BytePerPixel);
        // X corresponds to column.
        // int img_col = TileX * PageSize.x;
        std::ifstream in{StreamingPath.generic_string(), std::ios::binary};
        int MipWidth = Image->GetWidth() >> MipmapLevel;
        int MipHeight = Image->GetHeight() >> MipmapLevel;
        for (int row = 0; row < PageSize.y; row++)
        {
            // It's the byte offset from the beginning of file to the beginning of binary block
            size_t bin_offset = StreamingBufferOffset;

            // It's the byte offset from the beginning of binary block to the beginning of this tile
            size_t tile_offset = Tile->DataOffset;

            // It's the byte offset from the beginning of this tile to the beginning of this row
            size_t inner_offset = row * MipWidth * BytePerPixel;

            // It's the byte offset from the beginning of file to the beginning of this row
            size_t read_offset = bin_offset + tile_offset + inner_offset;

            in.seekg(read_offset, std::ios::beg);
            char* write_pointer = data.get() + row*PageSize.x*BytePerPixel;
            in.read(write_pointer, PageSize.x*BytePerPixel);
        }
        Tile->bCommited = true;  
        ENQUEUE_RENDER_COMMAND(UVirtualTexture_UpdateTile)(
            [this, TileX, TileY, MipmapLevel, data](FDynamicRHI* RHICmdList)
            {
                if (TextureResource && TextureResource->TextureRHI)
                {
                    RHICmdList->RHISparseTextureUpdateTile(
                        TextureResource->TextureRHI.get(), TileX, TileY, MipmapLevel, data.get());   
                }  
            });
    }

    void UVirtualTexture::Serialize(FArchive &Ar)
    {
        UTexture::Serialize(Ar);
        Ar.json["ClassName"] = "UVirtualTexture";
    }

    void UVirtualTexture::Deserialize(FArchive &Ar)
    {
        DeserializeWithoutImageData(Ar);
        StreamingPath = FPath::ContentDir().generic_string() + SerializationPath.generic_string();
        nlohmann::json &json = Ar.json;
        if (json["ClassName"] != "UVirtualTexture") return;

        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];
        
        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        if (texture_resource.contains("Image") == false) return;
        nlohmann::json image = texture_resource["Image"];
        StreamingBufferOffset = Ar.FileLength - Ar.BinLength;
        PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(GetTextureType(), Image->GetPixelFormat());
        BytePerTile = PageSize.x * PageSize.y * TranslatePixelFormatToBytePerPixel(Image->GetPixelFormat());
        LruCache.SetCapacity(MaxPhysicalMemoryByte / BytePerTile);
        
        NumTileX = glm::ceil(Image->GetWidth() / float(PageSize.x));
        NumTileY = glm::ceil(Image->GetHeight() / float(PageSize.y));

        Tiles.resize(TextureResource->NumMips);
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(Image->GetPixelFormat());
        uint32 MipmapOffset = 0;
        for (int MipmapLevel = 0; MipmapLevel < TextureResource->NumMips; MipmapLevel++)
        {
            int NumMipTileX = this->NumTileX >> MipmapLevel;
            int NumMipTileY = this->NumTileY >> MipmapLevel;
            Tiles[MipmapLevel].resize(NumMipTileX);
            for (int TileX = 0; TileX < NumMipTileX; TileX++)
            {
                for (int TileY = 0; TileY < NumMipTileY; TileY++)
                {
                    uint32 offset = MipmapOffset + (PageSize.y*TileY * PageSize.x*NumMipTileX + PageSize.x*TileX) * BytePerPixel;
                    std::unique_ptr<VirtualTextureTile> tile = std::make_unique<VirtualTextureTile>();
                    tile->TileX = TileX;
                    tile->TileY = TileY;
                    tile->MipmapLevel = MipmapLevel;
                    tile->DataOffset = offset;
                    Tiles[MipmapLevel][TileX].push_back(std::move(tile));
                }
            }
            MipmapOffset += NumMipTileX * NumMipTileY * PageSize.x * PageSize.y * BytePerPixel;
        }

        TextureResource->SetData(Image);
    }
}