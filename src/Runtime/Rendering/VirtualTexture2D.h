#pragma once
#include "Texture.h"

namespace nilou {

    class FVirtualTexture2DResource : public FTextureResource
    {
	public:

        FVirtualTexture2DResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_Texture2D;
        }
		
        virtual void InitRHI() override;

	    virtual FVirtualTexture2DResource* GetVirtualTexture2DResource() override { return this; }
	    virtual const FVirtualTexture2DResource* GetVirtualTexture2DResource() const override { return this; }
    };

    struct VirtualTextureTile
    {
        uint32 TileX;
        uint32 TileY;
        uint32 MipmapLevel;
        uint32 DataOffset;
        bool bCommited = false;
        std::mutex mutex;
    };

    class NCLASS UVirtualTexture : public UTexture
    {
        GENERATED_BODY()
    public:
        UVirtualTexture();

        virtual FTextureResource* CreateResource() override;

        void UpdateBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UpdateBoundSync(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTileSync(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        uvec2 GetNumTiles(int MipmapLevel=0) const { return uvec2(NumTileX >> MipmapLevel, NumTileY >> MipmapLevel); }

        ivec3 GetPageSize() const { return PageSize; }

        uint32 GetBytePerTile() const { return BytePerTile; }

        uint32 MaxPhysicalMemoryByte = 1024*1024*128;   // 128 MB physical memory limit for every virtual texture

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;

   private:

        void UpdateTileInternal(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        uint32 NumTileX;
        uint32 NumTileY;

        ivec3 PageSize;

        uint32 BytePerTile;

        std::vector<std::vector<std::vector<std::unique_ptr<VirtualTextureTile>>>> Tiles;

        TLruCache<VirtualTextureTile*, VirtualTextureTile*> LruCache;

        std::filesystem::path StreamingPath;
        // It's the offset from the beginning to the BIN block
        uint32 StreamingBufferOffset;

        BS::thread_pool thread_pool;

    };

}