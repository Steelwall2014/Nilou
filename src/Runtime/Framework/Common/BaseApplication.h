#pragma once
#include <memory>
#include "RenderingThread.h"
#include "Common/Scene.h"
#include "Common/World.h"
#include "Common/GfxConfiguration.h"
#include "Common/FrameSynchronizer.h"
#include "Common/ContentManager.h"
#include "Delegate.h"

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

        UWorld *GetWorld() { return World.get(); }
        FScene *GetScene() { return Scene.get(); }
        TMulticastDelegate<FDynamicRHI*> &GetPreRenderDelegate() { return PreRenderDelegate; }
        TMulticastDelegate<FDynamicRHI*> &GetPostRenderDelegate() { return PostRenderDelegate; }
        TMulticastDelegate<int, int> &GetScreenResizeDelegate() { return ScreenResizeDelegate; }
        FContentManager *GetContentManager() { return ContentManager.get(); }

    protected:
        float deltaTime = 0.0f;
        float accumTime = 0.0f;
        static std::atomic<bool> m_bQuit;
        GfxConfiguration m_Config;
        bool CursorEnabled = false;
        std::shared_ptr<UWorld> World;
        std::shared_ptr<FScene> Scene;
        std::unique_ptr<FContentManager> ContentManager;
        TMulticastDelegate<FDynamicRHI*> PreRenderDelegate;
        TMulticastDelegate<FDynamicRHI*> PostRenderDelegate;
        TMulticastDelegate<int, int> ScreenResizeDelegate;
        std::unique_ptr<FRunnableThread> RenderingThread;
        std::atomic<bool> RenderingThreadInitialized = false;
        std::atomic<bool> bShouldRenderingThreadExit = false;


    private:
        BaseApplication() {}
    };

    BaseApplication *GetAppication();
}