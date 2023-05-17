#pragma once
#include "Texture.h"

namespace nilou {

    class FTextureRenderTargetResource : public FTextureResource
    {
	public:

        FTextureRenderTargetResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureResource(InName, InTextureParams, InNumMips)
        { }
        
        virtual class FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() { return nullptr; }
        virtual class FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() { return nullptr; }

        vec3 ClearColor;

    };

    class FTextureRenderTarget2DResource : public FTextureRenderTargetResource
    {
	public:

        FTextureRenderTarget2DResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureRenderTargetResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_Texture2D;
        }
		
        virtual void InitRHI() override;
        
        virtual FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() { return this; }

        RHIFramebufferRef RenderTargetFramebuffer;
    };

    class FTextureRenderTargetCubeResource : public FTextureRenderTargetResource
    {
	public:

        FTextureRenderTargetCubeResource(const std::string& InName, const RHITextureParams& InTextureParams, int32 InNumMips=1)
            : FTextureRenderTargetResource(InName, InTextureParams, InNumMips)
        { 
            TextureType = ETextureType::TT_TextureCube;
        }
		
        virtual void InitRHI() override;
        
        virtual FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() { return this; }

        std::array<RHITexture2DRef, 6> RenderTargetTextureViews;

        std::array<RHIFramebufferRef, 6> RenderTargetFramebuffers;

    };

    class NCLASS UTextureRenderTarget : public UTexture
    {
        GENERATED_BODY()
    public:

        UTextureRenderTarget()
            : ClearColor(vec3(0))
        { }

        /**
         * Modify the value of ClearColor and then call UpdateResource()
         * to update clear color.
         */
        vec3 ClearColor;

        FTextureRenderTargetResource* GetRenderTargetResource();

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    };

    class NCLASS UTextureRenderTarget2D : public UTextureRenderTarget
    {
        GENERATED_BODY()
    public:

        UTextureRenderTarget2D()
        { }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;

    };

    class NCLASS UTextureRenderTargetCube : public UTextureRenderTarget
    {
        GENERATED_BODY()
    public:

        UTextureRenderTargetCube()
        { }

        virtual FTextureResource* CreateResource() override;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

    protected:

        virtual std::shared_ptr<FImage> CreateImage(const ImageCreateInfo& ImageInfo) override;

    };

}