#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture3DResource : public FTextureResource
    {
    public:

        FTexture3DResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureResource(InName, InSamplerState, InNumMips)
        { 
            TextureType = ETextureDimension::Texture3D;
        }

        virtual void InitRHI(RenderGraph&) override;

	    virtual FTexture3DResource* GetTexture3DResource() override { return this; }
	    virtual const FTexture3DResource* GetTexture3DResource() const override { return this; }
    };

    class NCLASS UTexture3D : public UTexture
    {
        GENERATED_BODY()
    public:

        virtual FTextureResource* CreateResource() override;

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;
    };
    
}