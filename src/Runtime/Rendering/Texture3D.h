#pragma once
#include "Texture.h"

namespace nilou {

    class FTexture3DResource : public FTextureResource
    {
    public:

        FTexture3DResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_Texture3D;
        }

        virtual void InitRHI() override;

	    virtual FTexture3DResource* GetTexture3DResource() override { return this; }
	    virtual const FTexture3DResource* GetTexture3DResource() const override { return this; }
    };

    UCLASS()
    class UTexture3D : public UTexture
    {
        GENERATE_CLASS_INFO()
    public:
        UTexture3D(const std::string &InName="")
            : UTexture(InName)
        {

        }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;
    };
    
}