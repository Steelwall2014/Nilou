#pragma once
namespace nilou {

    class FRendererModule
    {
    public:
        void BeginRenderingViewFamily(class FSceneViewFamily* ViewFamily);
    };

    FRendererModule* GetRendererModule();
}