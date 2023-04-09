#pragma once
#include "Platform.h"
#include "RHIResources.h"

namespace nilou {

    class FViewport
    {
    public:
        uint32 Width, Height;
        class FTextureRenderTargetResource* RenderTarget = nullptr;
    };

}