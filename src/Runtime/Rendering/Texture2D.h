#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture2DResource : public FTextureResource
    {
    public:

        FTexture2DResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureResource(InName, InSamplerState, InNumMips)
        { 
            TextureType = ETextureDimension::Texture2D;
        }

        virtual void InitRHI(RenderGraph&) override;

	    virtual FTexture2DResource* GetTexture2DResource() override { return this; }
	    virtual const FTexture2DResource* GetTexture2DResource() const override { return this; }
    };

    class NCLASS UTexture2D : public UTexture
    {
        GENERATED_BODY()
    public:
        UTexture2D()
        {

        }

        static UTexture2D* CreateTransient(std::string Name, int32 InSizeX, int32 InSizeY, EPixelFormat InFormat);

        virtual FTextureResource* CreateResource() override;

        virtual void ReadPixelsRenderThread(RHICommandListImmediate& RHICmdList) override;

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;

    };
    
}