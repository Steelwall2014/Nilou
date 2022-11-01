#pragma once
#include "Common/Scene.h"
#include "Common/World.h"
#include "Interface/IApplication.h"
#include "Common/GfxConfiguration.h"
#include <memory>

namespace nilou {
    class BaseApplication : implements IApplication
    {
    public:
        BaseApplication(GfxConfiguration &cfg);
        virtual int Initialize() override;
        virtual void Finalize() override;
        virtual void Tick(double) override;

        virtual bool IsQuit() override;
        virtual GfxConfiguration &GetConfiguration() override;
        virtual float GetTimeSinceStart() override;
        virtual void SetWindowWidth(int width) override;
        virtual void SetWindowHeight(int height) override;
        virtual bool IsCursorEnabled() override;

        UWorld *GetWorld() { return World.get(); }
        FScene *GetScene() { return Scene.get(); }

    protected:
        float deltaTime = 0.0f;
        float accumTime = 0.0f;
        static bool m_bQuit;
        GfxConfiguration m_Config;
        bool CursorEnabled = false;
        std::shared_ptr<UWorld> World;
        std::shared_ptr<FScene> Scene;

    private:
        BaseApplication() {}
    };

    extern BaseApplication *g_pApp;
}