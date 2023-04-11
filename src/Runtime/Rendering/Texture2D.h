#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture2DResource : public FTextureResource
    {
    public:

        FTexture2DResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_Texture2D;
        }

        virtual void InitRHI() override;

	    virtual FTexture2DResource* GetTexture2DResource() override { return this; }
	    virtual const FTexture2DResource* GetTexture2DResource() const override { return this; }
    };

    UCLASS()
    class UTexture2D : public UTexture
    {
        GENERATE_CLASS_INFO()
    public:
        UTexture2D(const std::string &InName="")
            : UTexture(InName)
        {

        }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        virtual void ReadPixelsRenderThread(FDynamicRHI* RHICmdList) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;

    };
    
}