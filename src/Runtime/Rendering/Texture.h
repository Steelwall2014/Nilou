#pragma once

// #include "Common/Image.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include <string>
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
		~FImage() { delete[] data; }
	};

    class FTexture : public FRenderResource
    {
	public:
		FTexture(const std::string &InName, int32 InNumMips, std::shared_ptr<FImage> InImage)
            : Name(InName)
            , NumMips(InNumMips)
            , Image(InImage)
        {

        }
		
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

        std::string GetTextureName()
        {
            return Name;
        }

    protected:
		std::shared_ptr<FImage> Image;
		std::string Name;
        int32 NumMips;
        FRHISampler SamplerRHI;
    };

}