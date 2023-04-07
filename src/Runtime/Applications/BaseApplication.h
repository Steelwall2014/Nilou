#pragma once
#include <memory>
#include "RenderingThread.h"
#include "Scene.h"
#include "Common/World.h"
#include "GfxConfiguration.h"
#include "Common/FrameSynchronizer.h"
#include "Common/ContentManager.h"
#include "Common/Delegate.h"
#include "Common/GameViewportClient.h"

namespace nilou {
    class BaseApplication
    {
    public:
        BaseApplication(GfxConfiguration &cfg);
        virtual bool Initialize();
        virtual bool Initialize_RenderThread() { }
        virtual void Finalize();
        virtual void Finalize_RenderThread() { }
        virtual void Tick(double);
        virtual void Tick_RenderThread();

        virtual bool IsQuit();
        virtual GfxConfiguration &GetConfiguration();
        virtual float GetTimeSinceStart();
        virtual void SetWindowWidth(int width);
        virtual void SetWindowHeight(int height);
        virtual bool IsCursorEnabled();

        bool ShouldRenderingThreadExit() const { return bShouldRenderingThreadExit; }

        UWorld *GetWorld() { return GameViewportClient->World.get(); }
        FScene *GetScene() { return GameViewportClient->Scene.get(); }
        TMulticastDelegate<FDynamicRHI*, FScene*> &GetPreRenderDelegate() { return PreRenderDelegate; }
        TMulticastDelegate<FDynamicRHI*, FScene*> &GetPostRenderDelegate() { return PostRenderDelegate; }
        TMulticastDelegate<int, int> &GetScreenResizeDelegate() { return ScreenResizeDelegate; }
        FContentManager *GetContentManager() { return ContentManager.get(); }

    protected:
        float deltaTime = 0.0f;
        float accumTime = 0.0f;
        static std::atomic<bool> m_bQuit;
        GfxConfiguration m_Config;
        bool CursorEnabled = false;
        std::unique_ptr<FContentManager> ContentManager;
        std::unique_ptr<UGameViewportClient> GameViewportClient;
        TMulticastDelegate<FDynamicRHI*, FScene*> PreRenderDelegate;
        TMulticastDelegate<FDynamicRHI*, FScene*> PostRenderDelegate;
        TMulticastDelegate<int, int> ScreenResizeDelegate;
        std::unique_ptr<FRunnableThread> RenderingThread;
        std::atomic<bool> RenderingThreadInitialized = false;
        std::atomic<bool> bShouldRenderingThreadExit = false;


    private:
        BaseApplication() {}
    };

    BaseApplication *GetAppication();
}