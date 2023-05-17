#pragma once
#include "Texture.h"

namespace nilou {

    class FTextureCubeResource : public FTextureResource
    {
    public:

        FTextureCubeResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_TextureCube;
        }

        virtual void InitRHI() override;

	    virtual FTextureCubeResource* GetTextureCubeResource() override { return this; }
	    virtual const FTextureCubeResource* GetTextureCubeResource() const override { return this; }
    };

    class NCLASS UTextureCube : public UTexture
    {
        GENERATED_BODY()
    public:
        UTextureCube()
        {

        }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;
    };
}