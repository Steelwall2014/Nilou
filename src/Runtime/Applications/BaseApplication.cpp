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
        RenderingThread = std::move(FRunnableThread::Create(new FRenderingThread, "Rendering Thread"));
        GameViewportClient = std::make_unique<UGameViewportClient>();
        while (!RenderingThread->IsRunnableInitialized()) { }
        GameViewportClient->Init();
        GameViewportClient->BeginPlay();
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
        GameViewportClient->Tick(DeltaTime);
        static FViewport Viewport;
        Viewport.Width = GetConfiguration().screenWidth;
        Viewport.Height = GetConfiguration().screenHeight;
        GameViewportClient->Draw(Viewport);
        ENQUEUE_RENDER_COMMAND(BaseApplication_Tick)(
            [this](FDynamicRHI*) 
            {
                this->Tick_RenderThread();
            });

        std::mutex m;
        std::unique_lock<std::mutex> lock(m);
        std::condition_variable fence;
        ENQUEUE_RENDER_COMMAND(BaseApplication_Fence)(
            [&fence](FDynamicRHI*) 
            {
                fence.notify_one();
            });
        fence.wait(lock);
    }

    void BaseApplication::Tick_RenderThread()
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

}
