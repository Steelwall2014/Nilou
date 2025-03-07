#include <magic_enum/magic_enum_utility.hpp>
#include <fstream>
#include "Common/Path.h"
#include "Common/Asset/AssetLoader.h"
#include "DynamicRHI.h"
#include "Common/Log.h"

#include "Texture2D.h"
#include "Texture3D.h"
#include "Texture2DArray.h"
#include "TextureCube.h"
#include "VirtualTexture2D.h"
#include "RenderGraph.h"

namespace nilou {

    void FTexture::ReleaseRHI()
    {
        TextureRDG = nullptr;
    }

    void FTexture::SetData(FImage* InImage)
    {
        Image = InImage;
    }

    // std::shared_ptr<UVirtualTexture> UTexture::MakeVirtualTexture()
    // {
    //     std::shared_ptr<UVirtualTexture> VT = std::make_shared<UVirtualTexture>();
    //     VT->Name = std::move(Name);
    //     VT->TextureResource->Name = std::move(this->TextureResource->Name);
    //     VT->TextureResource->NumMips = std::move(this->TextureResource->NumMips);
    //     VT->TextureResource->SamplerRHI.SamplerState = std::move(this->TextureResource->SamplerRHI.SamplerState);
    //     this->TextureResource = nullptr;
    //     return VT;
    // }

    void UTexture::ReadPixelsSync()
    {
        // std::mutex m;
        // std::unique_lock<std::mutex> lock(m);
        // std::condition_variable cv;
        // bool pixels_readed = false;
        // ENQUEUE_RENDER_COMMAND(UTexture_ReadPixelsSync)(
        //     [this, &cv, &pixels_readed](RHICommandList& RHICmdList)
        //     {
        //         ReadPixelsRenderThread(RHICmdList);
        //         cv.notify_one();
        //         pixels_readed = true;
        //     });
        // if (!pixels_readed)
        //     cv.wait(lock, [&pixels_readed] { return pixels_readed == true; });
    }

    void UTexture::PostDeserialize(FArchive& Ar)
    {
        if (GetResource() == nullptr)
        {
            // Sometimes the resource has been created in UMaterial::PostDeserialize
            // In this case, we don't need to create it again
            UpdateResource();
        }
    }

}