#pragma once
#include <half/half.hpp>
#include <string>
#include <thread_pool/BS_thread_pool.hpp>

#include "Common/CoreUObject/Object.h"
#include "Common/LruCache.h"

#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RenderResource.h"
namespace nilou {

	class FImage 
	{
	public:
        unsigned int Width{ 0 };
        unsigned int  Height{ 0 };
        unsigned int Channel{ 0 };
        unsigned char *data{ nullptr };
        size_t data_size{ 0 };
		EPixelFormat PixelFormat;
        void* Get(int row, int col);
		~FImage() { delete[] data; }
	};

    class FTexture : public FRenderResource
    {
	public:
		FTexture(int32 InNumMips=1, std::shared_ptr<FImage> InImage=nullptr)
            : NumMips(InNumMips)
            , Image(InImage)
        {

        }

        virtual ~FTexture() { ReleaseResource(); }
		
        virtual void InitRHI() override;
        virtual void ReleaseRHI() override;

        FRHISampler *GetSamplerRHI()
        {
            return &SamplerRHI;
        }

        void SetSamplerParams(const RHITextureParams &InTextureParams)
        {
            SamplerRHI.Params = InTextureParams;
        }

        void SetImage(std::shared_ptr<FImage> InImage)
        {
            Image = InImage;
            InitRHI();
        }

		std::shared_ptr<FImage> Image;
        int32 NumMips;
        FRHISampler SamplerRHI;
        RHITextureRef TextureRHI;
    };

    class FSparseTexture : public FTexture
    {
	public:
		FSparseTexture(int32 InNumMips=1, std::shared_ptr<FImage> InImage=nullptr)
            : FTexture(InNumMips, InImage)
        {

        }
		
        virtual void InitRHI() override;
    };

    UCLASS()
    class UTexture : public UObject
    {
        GENERATE_CLASS_INFO()
    public:
        UTexture()
            : Name("")
            , TextureResource(std::make_unique<FTexture>())
        {

        }

        UTexture(const std::string &InName, std::unique_ptr<FTexture> InTextureResource=nullptr)
            : Name(InName)
            , TextureResource(std::move(InTextureResource))
        {

        }

        UTexture(const std::string &InName, int32 InNumMips, std::shared_ptr<FImage> InImage)
            : Name(InName)
            , TextureResource(std::make_unique<FTexture>(InNumMips, InImage))
        {

        }

		std::string Name;

        ETextureWrapModes GetWrapS()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_S;
        }

        ETextureWrapModes GetWrapR()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_R;
        }

        ETextureWrapModes GetWrapT()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_T;
        }

        ETextureFilters GetMagFilter()
        {
            return TextureResource->GetSamplerRHI()->Params.Mag_Filter;
        }

        ETextureFilters GetMinFilter()
        {
            return TextureResource->GetSamplerRHI()->Params.Min_Filter;
        }

        ETextureType GetTextureType()
        {
            return TextureResource->GetSamplerRHI()->Texture->GetTextureType();
        }

        FTexture *GetResource()
        {
            return TextureResource.get();
        }

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        virtual void ReleaseRenderResources()
        {
            BeginReleaseResource(TextureResource.get());
        }
    
    protected:
        std::unique_ptr<FTexture> TextureResource;
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

    UCLASS()
    class UVirtualTexture : public UTexture
    {
        GENERATE_CLASS_INFO()
    public:
        UVirtualTexture();

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        void UpdateBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UpdateBoundSync(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadBound(vec2 UV_Min, vec2 UV_Max, uint32 MipmapLevel);

        void UnloadTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTile(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        void UpdateTileSync(uint32 TileX, uint32 TileY, uint32 MipmapLevel);

        uvec2 GetNumTiles() const { return uvec2(NumTileX, NumTileY); }

        ivec3 GetPageSize() const { return PageSize; }

        uint32 GetBytePerTile() const { return BytePerTile; }

        uint32 MaxPhysicalMemoryByte = 1024*1024*128;   // 128 MB physical memory limit for every virtual texture

   private:

        uint32 NumTileX;
        uint32 NumTileY;

        ivec3 PageSize;

        uint32 BytePerTile;

        std::vector<std::vector<std::vector<std::unique_ptr<VirtualTextureTile>>>> Tiles;

        TLruCache<VirtualTextureTile*, VirtualTextureTile*> LruCache;

        std::filesystem::path StreamingPath;
        uint32 StreamingBufferOffset;

        BS::thread_pool thread_pool;

    };

}