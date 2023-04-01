#include "BaseApplication.h"

#include "Common/Path.h"

namespace nilou {

    std::atomic<bool> BaseApplication::m_bQuit = false;

    BaseApplication::BaseApplication(GfxConfiguration &cfg) :
        m_Config(cfg)
    {
    }

    bool BaseApplication::Initialize()
    {
        m_bQuit = false;

        ContentManager = std::make_unique<FContentManager>(FPath::ContentDir());
        this->RenderingThread = std::move(FRunnableThread::Create(new FRenderingThread, "Rendering Thread"));
        
        World = std::make_shared<UWorld>();
        Scene = std::make_shared<FScene>();
        World->Scene = Scene.get();
        Scene->World = World.get();
        while (!RenderingThread->IsRunnableInitialized()) { }
        World->InitWorld();
        World->BeginPlay();
        return true;
    }


    void BaseApplication::Finalize()
    {
        ContentManager->Flush();
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

}
