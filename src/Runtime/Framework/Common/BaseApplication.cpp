#include <gdal.h>
#include <gdal_priv.h>

#include "GLFWApplication.h"

#include "Path.h"

namespace nilou {

    std::atomic<bool> BaseApplication::m_bQuit = false;

    BaseApplication::BaseApplication(GfxConfiguration &cfg) :
        m_Config(cfg)
    {
    }

    bool BaseApplication::Initialize()
    {
        m_bQuit = false;

        this->RenderingThread = std::move(FRunnableThread::Create(new FRenderingThread, "Rendering Thread"));
        
        World = std::make_shared<UWorld>();
        Scene = std::make_shared<FScene>();
        World->Scene = Scene.get();
        Scene->World = World.get();
        ContentManager = std::make_unique<FContentManager>(FPath::ContentDir());
        while (!RenderingThread->IsRunnableInitialized()) { }
        ContentManager->Init();
		GDALAllRegister();
        World->InitWorld();
        World->BeginPlay();
        return true;
    }


    void BaseApplication::Finalize()
    {
        ContentManager->ReleaseRenderResources();
        bShouldRenderingThreadExit = true;
        while (!RenderingThread->IsRunnableExited()) { }
    }


    void BaseApplication::Tick(double DeltaTime)
    {
        World->Tick(DeltaTime);

        if (Scene)
        {
            UWorld *World = Scene->World;
            if (World)
                World->SendAllEndOfFrameUpdates();
        }

        FFrameSynchronizer::MainThreadFrameCount++;
        if (FFrameSynchronizer::MainThreadFrameCount == 1)
        {
            FFrameSynchronizer::ShouldRenderingThreadLoopRun = true;
            FFrameSynchronizer::cv.notify_all();
        }
        if (FFrameSynchronizer::MainThreadFrameCount == FFrameSynchronizer::RenderingThreadFrameCount+1 || m_bQuit)
        {
            FFrameSynchronizer::ShouldRenderingThreadWait = false;
            FFrameSynchronizer::render_cv.notify_all();
        }
        else if (FFrameSynchronizer::MainThreadFrameCount > FFrameSynchronizer::RenderingThreadFrameCount+1)
        {
            FFrameSynchronizer::ShouldMainThreadWait = true;
            std::unique_lock<std::mutex> lock(FFrameSynchronizer::main_mutex);
            FFrameSynchronizer::main_cv.wait(lock, []() { return FFrameSynchronizer::ShouldMainThreadWait == false; });
        }
    }

    void BaseApplication::Tick_RenderThread()
    {
        FFrameSynchronizer::RenderingThreadFrameCount++;
        if (FFrameSynchronizer::RenderingThreadFrameCount == FFrameSynchronizer::MainThreadFrameCount-1 || m_bQuit)
        {
            FFrameSynchronizer::ShouldMainThreadWait = false;
            FFrameSynchronizer::main_cv.notify_all();
        }
        else if (FFrameSynchronizer::RenderingThreadFrameCount > FFrameSynchronizer::MainThreadFrameCount-1)
        {
            FFrameSynchronizer::ShouldRenderingThreadWait = true;
            std::unique_lock<std::mutex> lock(FFrameSynchronizer::render_mutex);
            FFrameSynchronizer::render_cv.wait(lock, []() { return FFrameSynchronizer::ShouldRenderingThreadWait == false; });
        }
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
