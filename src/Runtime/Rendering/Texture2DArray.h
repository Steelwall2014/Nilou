#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture2DArrayResource : public FTextureResource
    {
    public:

        FTexture2DArrayResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_Texture2DArray;
        }

        virtual void InitRHI() override;

	    virtual FTexture2DArrayResource* GetTexture2DArrayResource() override { return this; }
	    virtual const FTexture2DArrayResource* GetTexture2DArrayResource() const override { return this; }
    };

    class NCLASS UTexture2DArray : public UTexture
    {
        GENERATE_BODY()
    public:
        UTexture2DArray()
        {

        }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;
    };

}