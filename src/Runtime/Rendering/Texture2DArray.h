#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture2DArrayResource : public FTextureResource
    {
    public:

        FTexture2DArrayResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureResource(InName, InSamplerState, InNumMips)
        { 
            TextureType = ETextureDimension::Texture2DArray;
        }

        virtual void InitRHI(RenderGraph&) override;

	    virtual FTexture2DArrayResource* GetTexture2DArrayResource() override { return this; }
	    virtual const FTexture2DArrayResource* GetTexture2DArrayResource() const override { return this; }
    };

    class NCLASS UTexture2DArray : public UTexture
    {
        GENERATED_BODY()
    public:

        virtual FTextureResource* CreateResource() override;

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;
    };

}