#pragma once
#include <memory>
#include "RenderingThread.h"
#include "Common/Scene.h"
#include "Common/World.h"
#include "Common/GfxConfiguration.h"
#include "Delegate.h"

namespace nilou {
    class BaseApplication
    {
    public:
        BaseApplication(GfxConfiguration &cfg);
        virtual bool Initialize();
        virtual bool Initialize_RenderThread() { }
        virtual void Finalize();
        virtual void Tick(double);

        virtual bool IsQuit();
        virtual GfxConfiguration &GetConfiguration();
        virtual float GetTimeSinceStart();
        virtual void SetWindowWidth(int width);
        virtual void SetWindowHeight(int height);
        virtual bool IsCursorEnabled();

        UWorld *GetWorld() { return World.get(); }
        FScene *GetScene() { return Scene.get(); }
        TMulticastDelegate<FDynamicRHI*> &GetPreRenderDelegate() { return PreRenderDelegate; }

    protected:
        float deltaTime = 0.0f;
        float accumTime = 0.0f;
        static bool m_bQuit;
        GfxConfiguration m_Config;
        bool CursorEnabled = false;
        std::shared_ptr<UWorld> World;
        std::shared_ptr<FScene> Scene;
        TMulticastDelegate<FDynamicRHI*> PreRenderDelegate;
        std::unique_ptr<FRunnableThread> RenderingThread;
        std::atomic<bool> RenderingThreadInitialized;


    private:
        BaseApplication() {}
    };

    BaseApplication *GetAppication();
}