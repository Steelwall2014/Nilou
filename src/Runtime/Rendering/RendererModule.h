#pragma once
namespace nilou {

    class FSceneViewFamily;

    class FRendererModule
    {
    public:
        void BeginRenderingViewFamily(FSceneViewFamily* ViewFamily);
    };

    FRendererModule* GetRendererModule();
}