#include "BaseApplication.h"
#include "DynamicRHI.h"
#include "Common/Path.h"
#include "Common/Crc.h"

namespace nilou {

    std::atomic<bool> BaseApplication::m_bQuit = false;

    BaseApplication::BaseApplication(GfxConfiguration &cfg) :
        m_Config(cfg)
    {
    }

    bool BaseApplication::Initialize()
    {
        m_bQuit = false;

        FCrc::Init();
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
        bShouldRenderingThreadExit = true;
        while (!RenderingThread->IsRunnableExited()) { }
    }

    void BaseApplication::Finalize_RenderThread()
    {
        GameViewportClient->World = nullptr;
        GetScene()->Release_RenderThread();
        ContentManager->ReleaseRenderResources();
    }

    void BaseApplication::Tick(double DeltaTime)
    {
        ENQUEUE_RENDER_COMMAND(BaseApplication_BeginFrame)(
            [this](FDynamicRHI* RHICmdList) 
            {
                FRenderingThread::NotifyStartOfFrame();
                RHICmdList->RHIBeginFrame();
            });
        GameViewportClient->Tick(DeltaTime);
        static FViewport Viewport;
        Viewport.Width = GetConfiguration().screenWidth;
        Viewport.Height = GetConfiguration().screenHeight;
        GameViewportClient->Draw(Viewport);
        std::mutex m;
        std::unique_lock<std::mutex> lock(m);
        std::condition_variable fence;
        ENQUEUE_RENDER_COMMAND(BaseApplication_Tick)(
            [this, &fence](FDynamicRHI* RHICmdList) 
            {
                this->Tick_RenderThread();
                RHICmdList->RHIEndFrame();
                FRenderingThread::NotifyEndOfFrame();
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
