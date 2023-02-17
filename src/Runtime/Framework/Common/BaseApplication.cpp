#include "GLFWApplication.h"

namespace nilou {

    bool BaseApplication::m_bQuit = false;

    BaseApplication::BaseApplication(GfxConfiguration &cfg) :
        m_Config(cfg)
    {
    }

    bool BaseApplication::Initialize()
    {
        m_bQuit = false;

        this->RenderingThread = std::move(FRunnableThread::Create(new FRenderingThread, "Rendering Thread"));

        return true;
    }


    void BaseApplication::Finalize()
    {
    }


    void BaseApplication::Tick(double DeltaTime)
    {
    }

    bool BaseApplication::IsQuit()
    {
        return m_bQuit;
    }

    GfxConfiguration &BaseApplication::GetConfiguration()
    {
        return m_Config;
    }

    void BaseApplication::SetWindowWidth(int width)
    {
        m_Config.screenWidth = width;
    }

    void BaseApplication::SetWindowHeight(int height)
    {
        m_Config.screenHeight = height;
    }

    float BaseApplication::GetTimeSinceStart()
    {
        return accumTime;
    }

    bool BaseApplication::IsCursorEnabled()
    {
        return CursorEnabled;
    }

    BaseApplication *GetAppication()
    {
        static BaseApplication *g_pApp;
        if (g_pApp == nullptr)
        {
            GfxConfiguration config(8, 8, 8, 8, 32, 0, 0, 1600, 900, L"test");
            g_pApp = new GLFWApplication(config);
        }
        return g_pApp;
    }

}
