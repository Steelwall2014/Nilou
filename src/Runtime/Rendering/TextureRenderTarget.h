#pragma once
#include "Texture.h"

namespace nilou {

    class FTextureRenderTargetResource : public FTextureResource
    {
	public:

        FTextureRenderTargetResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureResource(InName, InSamplerState, InNumMips)
        { }
        
        virtual class FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() { return nullptr; }
        virtual class FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() { return nullptr; }

        vec3 ClearColor;

    };

    class FTextureRenderTarget2DResource : public FTextureRenderTargetResource
    {
	public:

        FTextureRenderTarget2DResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureRenderTargetResource(InName, InSamplerState, InNumMips)
        { 
            TextureType = ETextureDimension::Texture2D;
        }
		
        virtual void InitRHI(RenderGraph&) override;

        virtual void ReleaseRHI() override 
        { 

        }
        
        virtual FTextureRenderTarget2DResource* GetTextureRenderTarget2DResource() override { return this; }

        RDGRenderTargets RenderTarget;
    };

    class FTextureRenderTargetCubeResource : public FTextureRenderTargetResource
    {
	public:

        FTextureRenderTargetCubeResource(const std::string& InName, const FSamplerStateInitializer& InSamplerState, int32 InNumMips=1)
            : FTextureRenderTargetResource(InName, InSamplerState, InNumMips)
        { 
            TextureType = ETextureDimension::TextureCube;
        }
		
        virtual void InitRHI(RenderGraph&) override;

        virtual void ReleaseRHI() override 
        { 
            RenderTargetTextureViews.fill(nullptr);
        }
        
        virtual FTextureRenderTargetCubeResource* GetTextureRenderTargetCubeResource() override { return this; }

        std::array<RDGTextureRef, 6> RenderTargetTextureViews;

        std::array<RDGRenderTargets, 6> RenderTargetFramebuffers;

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
        NPROPERTY()
        vec3 ClearColor;

        FTextureRenderTargetResource* GetRenderTargetResource();

    };

    class NCLASS UTextureRenderTarget2D : public UTextureRenderTarget
    {
        GENERATED_BODY()
    public:

        UTextureRenderTarget2D()
        { }

        virtual FTextureResource* CreateResource() override;

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;

    };

    class NCLASS UTextureRenderTargetCube : public UTextureRenderTarget
    {
        GENERATED_BODY()
    public:

        UTextureRenderTargetCube()
        { }

        virtual FTextureResource* CreateResource() override;

    protected:

        virtual FImage CreateImage(const ImageCreateInfo& ImageInfo) override;

    };

}