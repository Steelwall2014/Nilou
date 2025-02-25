#include <fstream>

#include "VirtualTexture2D.h"
#include "Common/Path.h"
#if NILOU_ENABLE_VIRTUAL_TEXTURE

namespace nilou {
		
    void FVirtualTexture2DResource::InitRHI()
    {
        if (Image && Image->GetImageType() != EImageType::IT_Image2D)
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
            NumMips, Image->GetWidth(), Image->GetHeight(), TexCreate_Virtual);
        SamplerRHI.Texture = TextureRHI.get();
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
                if (GetResource() == nullptr)
                    return;
                if (Tile->bCommited && GetResource() && GetResource()->TextureRHI)
                {
                    RHICmdList->RHISparseTextureUnloadTile(
                        GetResource()->TextureRHI.get(), TileX, TileY, MipmapLevel);
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
        MipmapLevel = glm::clamp(MipmapLevel, 0u, uint32(GetResource()->NumMips-1));
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

        if (Tile->bCommited || GetResource() == nullptr)
            return;  
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(ImageData.GetPixelFormat());
        ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(GetTextureType(), ImageData.GetPixelFormat());

        std::shared_ptr<char[]> data = std::make_shared<char[]>(PageSize.x*PageSize.y*BytePerPixel);
        // X corresponds to column.
        // int img_col = TileX * PageSize.x;
        //std::ifstream in{StreamingPath.generic_string(), std::ios::binary};
        int MipWidth = ImageData.GetWidth() >> MipmapLevel;
        int MipHeight = ImageData.GetHeight() >> MipmapLevel;
        for (int row = 0; row < PageSize.y; row++)
        {
            // It's the byte offset from the beginning of file to the beginning of binary block
            size_t bin_offset = 0;//StreamingBufferOffset;

            // It's the byte offset from the beginning of binary block to the beginning of this tile
            size_t tile_offset = Tile->DataOffset;

            // It's the byte offset from the beginning of this tile to the beginning of this row
            size_t inner_offset = row * MipWidth * BytePerPixel;

            // It's the byte offset from the beginning of file to the beginning of this row
            size_t read_offset = bin_offset + tile_offset + inner_offset;

            //in.seekg(read_offset, std::ios::beg);
            uint8* in = ImageData.GetData() + read_offset;
            char* write_pointer = data.get() + row*PageSize.x*BytePerPixel;
            std::memcpy(write_pointer, in, PageSize.x*BytePerPixel);
            //in.read(write_pointer, PageSize.x*BytePerPixel);
        }
        Tile->bCommited = true;  
        ENQUEUE_RENDER_COMMAND(UVirtualTexture_UpdateTile)(
            [this, TileX, TileY, MipmapLevel, data](FDynamicRHI* RHICmdList)
            {
                if (GetResource() && GetResource()->TextureRHI)
                {
                    RHICmdList->RHISparseTextureUpdateTile(
                        GetResource()->TextureRHI.get(), TileX, TileY, MipmapLevel, data.get());   
                }  
            });
    }

    void UVirtualTexture::PostDeserialize(FArchive& Ar)
    {
        //StreamingPath = FPath::ContentDir().generic_string() + SerializationPath.generic_string();
        //StreamingBufferOffset = Ar.FileLength - Ar.BinLength;
        PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(ETextureDimension::Texture2D, ImageData.GetPixelFormat());
        BytePerTile = PageSize.x * PageSize.y * TranslatePixelFormatToBytePerPixel(ImageData.GetPixelFormat());
        LruCache.SetCapacity(MaxPhysicalMemoryByte / BytePerTile);
        
        NumTileX = glm::ceil(ImageData.GetWidth() / float(PageSize.x));
        NumTileY = glm::ceil(ImageData.GetHeight() / float(PageSize.y));

        Tiles.resize(NumMips);
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(ImageData.GetPixelFormat());
        uint32 MipmapOffset = 0;
        for (int MipmapLevel = 0; MipmapLevel < NumMips; MipmapLevel++)
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

        UpdateResource();
    }

    // void UVirtualTexture::Serialize(FArchive &Ar)
    // {
    //     ImageData->AllocateSpace();
    //     std::ifstream in{StreamingPath.generic_string(), std::ios::binary};
    //     in.seekg(StreamingBufferOffset, std::ios::beg);
    //     in.read((char*)ImageData->GetData(), ImageData->GetDataSize());
    //     UTexture::Serialize(Ar);
    //     Ar.json["ClassName"] = "UVirtualTexture";
    // }

    // void UVirtualTexture::Deserialize(FArchive &Ar)
    // {
    //     UTexture::Deserialize(Ar);
    //     StreamingPath = FPath::ContentDir().generic_string() + SerializationPath.generic_string();
    //     StreamingBufferOffset = Ar.FileLength - Ar.BinLength;
    //     PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(ETextureDimension::Texture2D, ImageData->GetPixelFormat());
    //     BytePerTile = PageSize.x * PageSize.y * TranslatePixelFormatToBytePerPixel(ImageData->GetPixelFormat());
    //     LruCache.SetCapacity(MaxPhysicalMemoryByte / BytePerTile);
        
    //     NumTileX = glm::ceil(ImageData->GetWidth() / float(PageSize.x));
    //     NumTileY = glm::ceil(ImageData->GetHeight() / float(PageSize.y));

    //     Tiles.resize(NumMips);
    //     uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(ImageData->GetPixelFormat());
    //     uint32 MipmapOffset = 0;
    //     for (int MipmapLevel = 0; MipmapLevel < NumMips; MipmapLevel++)
    //     {
    //         int NumMipTileX = this->NumTileX >> MipmapLevel;
    //         int NumMipTileY = this->NumTileY >> MipmapLevel;
    //         Tiles[MipmapLevel].resize(NumMipTileX);
    //         for (int TileX = 0; TileX < NumMipTileX; TileX++)
    //         {
    //             for (int TileY = 0; TileY < NumMipTileY; TileY++)
    //             {
    //                 uint32 offset = MipmapOffset + (PageSize.y*TileY * PageSize.x*NumMipTileX + PageSize.x*TileX) * BytePerPixel;
    //                 std::unique_ptr<VirtualTextureTile> tile = std::make_unique<VirtualTextureTile>();
    //                 tile->TileX = TileX;
    //                 tile->TileY = TileY;
    //                 tile->MipmapLevel = MipmapLevel;
    //                 tile->DataOffset = offset;
    //                 Tiles[MipmapLevel][TileX].push_back(std::move(tile));
    //             }
    //         }
    //         MipmapOffset += NumMipTileX * NumMipTileY * PageSize.x * PageSize.y * BytePerPixel;
    //     }

    //     UpdateResource();
    // }

    FTextureResource* UVirtualTexture::CreateResource()
    {
        FVirtualTexture2DResource* Resource = new FVirtualTexture2DResource(Name, TextureParams, NumMips);
        Resource->SetData(&ImageData);
        return Resource;
    }

    FImage UVirtualTexture::CreateImage(const ImageCreateInfo& ImageInfo)
    {
        FImage image = FImage(
            ImageInfo.Width, ImageInfo.Height, 
            ImageInfo.PixelFormat, EImageType::IT_Image2D, ImageInfo.NumMips);
        return image;
    }

}
#endif