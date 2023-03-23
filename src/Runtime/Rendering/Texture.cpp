#include <magic_enum.hpp>
#include <fstream>
#include "Common/Path.h"
#include "Texture.h"
#include "Common/Asset/AssetLoader.h"
#include "DynamicRHI.h"
#include "RenderingThread.h"

namespace nilou {
		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D(Name, Image->PixelFormat, NumMips, Image->Width, Image->Height, Image->data);
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTexture::ReleaseRHI()
    {
        TextureRHI = nullptr;
    }
		
    void FSparseTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateSparseTexture2D(Name, Image->PixelFormat, NumMips, Image->Width, Image->Height);
        SamplerRHI.Texture = TextureRHI.get();
    }

    std::shared_ptr<UVirtualTexture> UTexture::MakeVirtualTexture()
    {
        std::shared_ptr<UVirtualTexture> VT = std::make_shared<UVirtualTexture>();
        VT->Name = std::move(Name);
        VT->TextureResource->Name = std::move(this->TextureResource->Name);
        VT->TextureResource->Image = std::move(this->TextureResource->Image);
        VT->TextureResource->NumMips = std::move(this->TextureResource->NumMips);
        VT->TextureResource->SamplerRHI.Params = std::move(this->TextureResource->SamplerRHI.Params);
        VT->TextureResource->SamplerRHI.ResourceType = std::move(this->TextureResource->SamplerRHI.ResourceType);
        if (TextureResource->TextureRHI)
            BeginInitResource(VT->TextureResource.get());
        this->TextureResource = nullptr;
        return VT;
    }

    void UTexture::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UTexture";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        if (TextureResource != nullptr)
        {
            nlohmann::json &texture_resource = content["TextureResource"];
            texture_resource["NumMips"] = TextureResource->NumMips;
            texture_resource["Wrap_S"] = magic_enum::enum_name(GetWrapS());
            texture_resource["Wrap_R"] = magic_enum::enum_name(GetWrapR());
            texture_resource["Wrap_T"] = magic_enum::enum_name(GetWrapT());
            texture_resource["MagFilter"] = magic_enum::enum_name(GetMagFilter());
            texture_resource["MinFilter"] = magic_enum::enum_name(GetMinFilter());
            // texture_resource["TextureType"] = magic_enum::enum_name(GetTextureType());
            nlohmann::json &image = texture_resource["Image"];
            image["Width"] = TextureResource->Image->Width;
            image["Height"] = TextureResource->Image->Height;
            image["Channel"] = TextureResource->Image->Channel;
            image["DataSize"] = TextureResource->Image->data_size;
            image["PixelFormat"] = magic_enum::enum_name(TextureResource->Image->PixelFormat);
            // image["Data"] = SerializeHelper::Base64Encode(TextureResource->Image->data, TextureResource->Image->data_size);
            Ar.OutBuffers.AddBuffer(image["Data"], TextureResource->Image->data_size, TextureResource->Image->data);
            
        }
    }

    void UTexture::Deserialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        if (json["ClassName"] != "UTexture") return;

        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];

        if (content.contains("Name") == false) return;
        Name = content["Name"];
        
        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        if (texture_resource.contains("Image") == false) return;
        nlohmann::json image = texture_resource["Image"];
        std::shared_ptr<FImage> Image = std::make_shared<FImage>();
        Image->Width = image["Width"];
        Image->Height = image["Height"];
        Image->Channel = image["Channel"];
        Image->data_size = image["DataSize"];
        Image->PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
        uint32 BufferOffset = image["Data"]["BufferOffset"];
        // std::string data = SerializeHelper::Base64Decode(image["Data"].get<std::string>());
        Image->data = new unsigned char[Image->data_size];
        std::memcpy(Image->data, Ar.InBuffer.get()+BufferOffset, Image->data_size);
        
        int NumMips = 1;
        if (texture_resource.contains("NumMips"))
            NumMips = texture_resource["NumMips"];
        TextureResource->Image = Image;
        TextureResource->NumMips = NumMips;
        TextureResource->Name = Name;

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
            
        // if (texture_resource.contains("TextureType"))
            // Params.TextureType = magic_enum::enum_cast<ETextureType>(std::string(texture_resource["TextureType"])).value();

        BeginInitResource(TextureResource.get());
    }

    uint32 TileToAddress()
    {

    }

    UVirtualTexture::UVirtualTexture()
        : LruCache(0)
    {
        TextureResource = std::make_unique<FTexture>();
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
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(TextureResource->Image->PixelFormat);
        ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(GetTextureType(), TextureResource->Image->PixelFormat);

        std::shared_ptr<char[]> data = std::make_shared<char[]>(PageSize.x*PageSize.y*BytePerPixel);
        // X corresponds to column.
        // int img_col = TileX * PageSize.x;
        std::ifstream in{StreamingPath.generic_string(), std::ios::binary};
        int MipWidth = TextureResource->Image->Width >> MipmapLevel;
        int MipHeight = TextureResource->Image->Height >> MipmapLevel;
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

            // int img_row = row+TileY*PageSize.y;
            // int img_col = TileX * PageSize.x;
            // size_t read_offset = StreamingBufferOffset + (img_row * TextureResource->Image->Width + img_col) * BytePerPixel;
            // if (read_offset0 != read_offset)
            //     std::cout << 1;

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
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UVirtualTexture";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        if (TextureResource != nullptr)
        {
            nlohmann::json &texture_resource = content["TextureResource"];
            texture_resource["NumMips"] = TextureResource->NumMips;
            texture_resource["Wrap_S"] = magic_enum::enum_name(GetWrapS());
            texture_resource["Wrap_R"] = magic_enum::enum_name(GetWrapR());
            texture_resource["Wrap_T"] = magic_enum::enum_name(GetWrapT());
            texture_resource["MagFilter"] = magic_enum::enum_name(GetMagFilter());
            texture_resource["MinFilter"] = magic_enum::enum_name(GetMinFilter());
            // texture_resource["TextureType"] = magic_enum::enum_name(GetTextureType());
            nlohmann::json &image = texture_resource["Image"];
            image["Width"] = TextureResource->Image->Width;
            image["Height"] = TextureResource->Image->Height;
            image["Channel"] = TextureResource->Image->Channel;
            image["DataSize"] = TextureResource->Image->data_size;
            image["PixelFormat"] = magic_enum::enum_name(TextureResource->Image->PixelFormat);
            if (TextureResource->Image->data)
                Ar.OutBuffers.AddBuffer(image["Data"], TextureResource->Image->data_size, TextureResource->Image->data);
        }
    }

    void UVirtualTexture::Deserialize(FArchive &Ar)
    {
        StreamingPath = FPath::ContentDir().generic_string() + SerializationPath.generic_string();
        nlohmann::json &json = Ar.json;
        if (json["ClassName"] != "UVirtualTexture") return;

        if (json.contains("Content") == false) return;
        nlohmann::json content = json["Content"];

        if (content.contains("Name") == false) return;
        Name = content["Name"];
        
        if (content.contains("TextureResource") == false) return;
        nlohmann::json texture_resource = content["TextureResource"];

        if (texture_resource.contains("Image") == false) return;
        nlohmann::json image = texture_resource["Image"];
        std::shared_ptr<FImage> Image = std::make_shared<FImage>();
        Image->Width = image["Width"];
        Image->Height = image["Height"];
        Image->Channel = image["Channel"];
        Image->data_size = image["DataSize"];
        Image->PixelFormat = magic_enum::enum_cast<EPixelFormat>(std::string(image["PixelFormat"])).value();
        // uint32 BufferOffset = image["Data"]["BufferOffset"];
        StreamingBufferOffset = Ar.FileLength - Ar.BinLength;
        // Image->data = new unsigned char[Image->data_size];
        // std::memcpy(Image->data, Ar.InBuffer.get()+BufferOffset, Image->data_size);
        // 暂时是写死成texture2d
        PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(ETextureType::TT_Texture2D, Image->PixelFormat);
        BytePerTile = PageSize.x * PageSize.y * TranslatePixelFormatToBytePerPixel(Image->PixelFormat);
        LruCache.SetCapacity(MaxPhysicalMemoryByte / BytePerTile);
        
        NumTileX = glm::ceil(Image->Width / float(PageSize.x));
        NumTileY = glm::ceil(Image->Height / float(PageSize.y));
        
        int NumMips = 1;
        if (texture_resource.contains("NumMips"))
            NumMips = texture_resource["NumMips"];
        TextureResource->Image = Image;
        TextureResource->NumMips = NumMips;
        TextureResource->Name = Name;

        Tiles.resize(NumMips);
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(Image->PixelFormat);
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
            
        // if (texture_resource.contains("TextureType"))
            // Params.TextureType = magic_enum::enum_cast<ETextureType>(std::string(texture_resource["TextureType"])).value();

        BeginInitResource(TextureResource.get());    
    }

    void* FImage::Get(int row, int col, int mipmap)
    {
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
        size_t mip0_size = Width*Height*Channel * BytePerPixel;
        size_t mip_offset = mip0_size * (1 - glm::pow(0.25, mipmap)) / (1 - 0.25);
        int mip_width = Width / glm::pow(2, mipmap);
        size_t offset = (row * mip_width + col) * BytePerPixel;
        if (mip_offset + offset < data_size)
            return data + mip_offset + offset;
        return nullptr;
    }
}