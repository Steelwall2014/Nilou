#pragma once
#include <memory>
#include "RenderingThread.h"
#include "Scene.h"
#include "Engine/World.h"
#include "GfxConfiguration.h"
#include "Delegate.h"
#include "GameViewportClient.h"

namespace nilou {

    using WindowContext = void;

    class ENGINE_API BaseApplication
    {
    public:
        BaseApplication();
        virtual bool Initialize();
        virtual bool Initialize_RenderThread();
        virtual void Finalize();
        virtual void Finalize_RenderThread();
        virtual void ProcessInput() { }
        virtual void Tick(double);
        virtual void Tick_RenderThread();

        virtual bool IsQuit();
        virtual GfxConfiguration &GetConfiguration();
        virtual float GetTimeSinceStart();
        virtual void SetWindowWidth(int width);
        virtual void SetWindowHeight(int height);
        virtual bool IsCursorEnabled();
        virtual void SetCursorCaptured(bool bCaptured) {}

        virtual WindowContext* GetWindowContext() { return nullptr; }

        UWorld *GetWorld() { return GameViewportClient->World; }
        FScene *GetScene() { return GameViewportClient->Scene.get(); }
        TMulticastDelegate<FDynamicRHI*, FScene*> &GetPreRenderDelegate() { return PreRenderDelegate; }
        TMulticastDelegate<FDynamicRHI*, FScene*> &GetPostRenderDelegate() { return PostRenderDelegate; }
        TMulticastDelegate<int, int> &GetScreenResizeDelegate() { return ScreenResizeDelegate; }
        static std::atomic<bool> m_bQuit;

    protected:
        float deltaTime = 0.0f;
        float accumTime = 0.0f;
        bool CursorEnabled = true;
        std::unique_ptr<UGameViewportClient> GameViewportClient;
        TMulticastDelegate<FDynamicRHI*, FScene*> PreRenderDelegate;
        TMulticastDelegate<FDynamicRHI*, FScene*> PostRenderDelegate;
        TMulticastDelegate<int, int> ScreenResizeDelegate;

    };

    ENGINE_API extern BaseApplication *GApplication;
}