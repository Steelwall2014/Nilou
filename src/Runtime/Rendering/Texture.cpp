#include <magic_enum.hpp>
#include "Texture.h"
#include "Common/AssetLoader.h"
#include "DynamicRHI.h"
#include "RenderingThread.h"

namespace nilou {


    static uint8 TranslatePixelFormatToBytePerPixel(EPixelFormat PixelFormat)
    {
        switch (PixelFormat) {
		    case EPixelFormat::PF_UNKNOWN: return 0;
		    case EPixelFormat::PF_R8: return 1;
		    case EPixelFormat::PF_R8G8: return 2;
		    case EPixelFormat::PF_R8G8B8: return 3;
		    case EPixelFormat::PF_R8G8B8_sRGB: return 3;
		    case EPixelFormat::PF_B8G8R8: return 3;
		    case EPixelFormat::PF_B8G8R8_sRGB: return 3;
		    case EPixelFormat::PF_R8G8B8A8: return 4;
		    case EPixelFormat::PF_R8G8B8A8_sRGB: return 4;
		    case EPixelFormat::PF_B8G8R8A8: return 4;
		    case EPixelFormat::PF_B8G8R8A8_sRGB: return 4;

		    case EPixelFormat::PF_D24S8: return 4;
		    case EPixelFormat::PF_D32F: return 4;
		    case EPixelFormat::PF_D32FS8: return 5;

		    case EPixelFormat::PF_DXT1: return 4;
		    case EPixelFormat::PF_DXT1_sRGB: return 4;
		    case EPixelFormat::PF_DXT5: return 4;
		    case EPixelFormat::PF_DXT5_sRGB: return 4;

		    case EPixelFormat::PF_R16F: return 2;
		    case EPixelFormat::PF_R16G16F: return 4;
		    case EPixelFormat::PF_R16G16B16F: return 6;
		    case EPixelFormat::PF_R16G16B16A16F: return 8;
		    case EPixelFormat::PF_R32F: return 4;
		    case EPixelFormat::PF_R32G32F: return 8;
		    case EPixelFormat::PF_R32G32B32F: return 12;
		    case EPixelFormat::PF_R32G32B32A32F: return 16;
            default: NILOU_LOG(Error, "Unknown PixelFormat: {}", (int)PixelFormat) return 0;
        }
    }
		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateTexture2D("", Image->PixelFormat, NumMips, Image->Width, Image->Height, Image->data);
        SamplerRHI.Texture = TextureRHI.get();
    }

    void FTexture::ReleaseRHI()
    {
        TextureRHI = nullptr;
    }
		
    void FSparseTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        TextureRHI = FDynamicRHI::GetDynamicRHI()->RHICreateSparseTexture2D("", Image->PixelFormat, NumMips, Image->Width, Image->Height);
        SamplerRHI.Texture = TextureRHI.get();
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
    {
        TextureResource = std::make_unique<FSparseTexture>();
    }

    void UVirtualTexture::UpdateBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel)
    {
        uint32 NumTileX = this->NumTileX >> MipmapLevel;
        uint32 NumTileY = this->NumTileY >> MipmapLevel;
        uvec2 tile_min = UV_Min * vec2(NumTileX, NumTileY);
        uvec2 tile_max = UV_Max * vec2(NumTileX, NumTileY);
        for (int i = tile_min.x; i < tile_max.x; i++)
        {
            for (int j = tile_min.y; j < tile_max.y; j++)
            {
                UpdateTile(i, j, MipmapLevel);
            }
        }
    }

    void UVirtualTexture::UnloadTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel)
    {
        ENQUEUE_RENDER_COMMAND(UVirtualTexture_UnloadTile)(
            [this, TileX, TileY, MipmapLevel](FDynamicRHI* RHICmdList)
            {
                auto &Tile = Tiles[MipmapLevel][TileX][TileY];
                if (Tile.bCommited)
                {
                    RHICmdList->RHISparseTextureUnloadTile(
                        TextureResource->TextureRHI.get(), TileX, TileY, MipmapLevel);
                    Tile.bCommited = false;
                }
            });
    }

    void UVirtualTexture::UpdateTile(uint32 InTileX, uint32 InTileY, uint32 MipmapLevel)
    {
        ENQUEUE_RENDER_COMMAND(UVirtualTexture_UnloadTile)(
            [this, InTileX, InTileY, MipmapLevel](FDynamicRHI* RHICmdList)
            {
                uint32 TileX = InTileX;
                uint32 TileY = InTileY;

                uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(TextureResource->Image->PixelFormat);
                ivec3 PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(GetTextureType(), TextureResource->Image->PixelFormat);
                auto &Tile = Tiles[MipmapLevel][TileX][TileY];
                LruUpdate(&Tile);
                if (!Tile.bCommited)
                {
                    std::unique_ptr<uint8[]> data = std::make_unique<uint8[]>(PageSize.x*PageSize.y*BytePerPixel);
                    // X corresponds to column.
                    int col = TileX * PageSize.x;
                    for (int row = 0; row < PageSize.y; row++)
                    {
                        std::memcpy(data.get()+row*PageSize.x*BytePerPixel, TextureResource->Image->Get(row+TileY*PageSize.y, col), PageSize.x*BytePerPixel);
                    }

                    RHICmdList->RHISparseTextureUpdateTile(
                        TextureResource->TextureRHI.get(), TileX, TileY, MipmapLevel, data.get());
                    Tile.bCommited = true;
                }
                
            });
    }

    void UVirtualTexture::LruUpdate(VirtualTextureTile *Tile)
    {
        if (TileToIterMap.find(Tile) != TileToIterMap.end())
        {
            auto iter = TileToIterMap[Tile];
            *iter = Tile;
            LruMoveToFront(iter);
        }
        else 
        {
            auto iter = LoadedTiles.insert(LoadedTiles.begin(), Tile);
            TileToIterMap[Tile] = iter;
            if (LoadedTiles.size()*BytePerTile > MaxPhysicalMemoryByte)
            {
                iter = LoadedTiles.end();
                iter--;
                VirtualTextureTile* Tile = *iter;
                LoadedTiles.erase(iter);
                UnloadTile(Tile->TileX, Tile->TileY, Tile->MipmapLevel);
            }
        }
    }

    void UVirtualTexture::LruMoveToFront(std::list<VirtualTextureTile*>::iterator iter)
    {
        if (iter != LoadedTiles.begin())
            LoadedTiles.splice(LoadedTiles.begin(), LoadedTiles, iter, std::next(iter));
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
            //  = {"Buffer"};//SerializeHelper::Base64Encode(TextureResource->Image->data, TextureResource->Image->data_size);
            Ar.OutBuffers.AddBuffer(image["Data"], TextureResource->Image->data_size, TextureResource->Image->data);
        }
    }

    void UVirtualTexture::Deserialize(FArchive &Ar)
    {
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
        uint32 BufferOffset = image["Data"]["BufferOffset"];
        // std::string data = SerializeHelper::Base64Decode(image["Data"].get<std::string>());
        Image->data = new unsigned char[Image->data_size];
        std::memcpy(Image->data, Ar.InBuffer.get()+BufferOffset, Image->data_size);
        // 暂时是写死成texture2d
        PageSize = FDynamicRHI::RHIGetSparseTexturePageSize(ETextureType::TT_Texture2D, Image->PixelFormat);
        BytePerTile = PageSize.x * PageSize.y * TranslatePixelFormatToBytePerPixel(Image->PixelFormat);
        NumTileX = glm::ceil(Image->Width / float(PageSize.x));
        NumTileY = glm::ceil(Image->Height / float(PageSize.y));
        
        int NumMips = 1;
        if (texture_resource.contains("NumMips"))
            NumMips = texture_resource["NumMips"];
        TextureResource->Image = Image;
        TextureResource->NumMips = NumMips;

        Tiles.resize(NumMips);
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(Image->PixelFormat);
        uint32 MipmapOffset = 0;
        for (int MipmapLevel = 0; MipmapLevel < NumMips; MipmapLevel++)
        {
            Tiles[MipmapLevel].resize(NumTileX);
            int NumTileX = this->NumTileX >> MipmapLevel;
            int NumTileY = this->NumTileY >> MipmapLevel;
            for (int TileX = 0; TileX < NumTileX; TileX++)
            {
                Tiles[MipmapLevel][TileX].resize(NumTileY);
                for (int TileY = 0; TileY < NumTileY; TileY++)
                {
                    uint32 offset = MipmapOffset + (PageSize.x*TileX * NumTileY + PageSize.y*TileY) * BytePerPixel;
                    VirtualTextureTile tile;
                    tile.TileX = TileX;
                    tile.TileY = TileY;
                    tile.MipmapLevel = MipmapLevel;
                    tile.DataOffset = offset;
                    Tiles[MipmapLevel][TileX][TileY] = tile;
                }
            }
            MipmapOffset += NumTileX * NumTileY * PageSize.x * PageSize.y * BytePerPixel;
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

    void* FImage::Get(int row, int col)
    {
        uint8 BytePerPixel = TranslatePixelFormatToBytePerPixel(PixelFormat);
        return data + (row * Width + col) * BytePerPixel;
    }
}