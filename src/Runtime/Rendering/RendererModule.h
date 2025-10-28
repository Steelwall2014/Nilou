#pragma once
namespace nilou {

    class FSceneViewFamily;

    class FRendererModule
    {
    public:
        void BeginRenderingViewFamily(const FSceneViewFamily& ViewFamily);
    };

    FRendererModule* GetRendererModule();
}