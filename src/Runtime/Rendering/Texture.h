#pragma once

#include "Common/CoreUObject/Object.h"

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
        friend class UTexture;
	public:
		FTexture(int32 InNumMips=1, std::shared_ptr<FImage> InImage=nullptr)
            : NumMips(InNumMips)
            , Image(InImage)
        {

        }

        ~FTexture() { ReleaseResource(); }
		
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

    protected:
		std::shared_ptr<FImage> Image;
        int32 NumMips;
        FRHISampler SamplerRHI;
        RHITextureRef TextureRHI;
    };

    UCLASS()
    class UTexture : public UObject
    {
        GENERATE_CLASS_INFO()
    public:
        UTexture()
            : Name("")
            , TextureResource(std::make_unique<FTexture>())
        {

        }

        UTexture(const std::string &InName, std::unique_ptr<FTexture> InTextureResource=nullptr)
            : Name(InName)
            , TextureResource(std::move(InTextureResource))
        {

        }

        UTexture(const std::string &InName, int32 InNumMips, std::shared_ptr<FImage> InImage)
            : Name(InName)
            , TextureResource(std::make_unique<FTexture>(InNumMips, InImage))
        {

        }

		std::string Name;

        ETextureWrapModes GetWrapS()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_S;
        }

        ETextureWrapModes GetWrapR()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_R;
        }

        ETextureWrapModes GetWrapT()
        {
            return TextureResource->GetSamplerRHI()->Params.Wrap_T;
        }

        ETextureFilters GetMagFilter()
        {
            return TextureResource->GetSamplerRHI()->Params.Mag_Filter;
        }

        ETextureFilters GetMinFilter()
        {
            return TextureResource->GetSamplerRHI()->Params.Min_Filter;
        }

        ETextureType GetTextureType()
        {
            return TextureResource->GetSamplerRHI()->Texture->GetTextureType();
        }

        FTexture *GetResource()
        {
            return TextureResource.get();
        }

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;
    
    protected:
        std::unique_ptr<FTexture> TextureResource;
    };

}