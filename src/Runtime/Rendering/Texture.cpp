#include "Texture.h"
#include "Common/AssetLoader.h"
#include "DynamicRHI.h"

namespace nilou {

		
    void FTexture::InitRHI()
    {
        FRenderResource::InitRHI();
        SamplerRHI.Texture = GDynamicRHI->RHICreateTexture2D(Name, Image->PixelFormat, NumMips, Image->Width, Image->Height, Image->data);
    }

    void FTexture::ReleaseRHI()
    {

    }

    // FTexture::FTexture()
    // {
    // }

    // FTexture::FTexture(std::shared_ptr<Image> img)
    // {
    //     m_Image = img;
    // }

    // FTexture::FTexture(const char *filepath)
    // {
    //     LoadTexture(filepath);
    // }

    // FTexture::FTexture(const std::string filepath)
    // {
    //     LoadTexture(filepath.c_str());
    // }

    // bool FTexture::LoadTexture(const char *filepath)
    // {
    //     m_Image = g_pAssetLoader->SyncOpenAndReadImage(filepath);
    //     return true;
    // }

    // std::shared_ptr<Image> FTexture::GetTextureImage()
    // {
    //     return m_Image;
    // }

}