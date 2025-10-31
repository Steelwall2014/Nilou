#include "BaseApplication.h"
#include "CoreMinimal.h"
#include "RHICommandList.h"
#include "Common/Path.h"
#include "Common/Crc.h"
#include "Common/CoreUObject/Serialization/AsyncLoading.h"

namespace nilou {

    std::atomic<bool> BaseApplication::m_bQuit = false;
    
    FRunnable* GRenderingThreadRunnable = nullptr;
    FRunnable* GAsyncLoadingThreadRunnable = nullptr;
    std::unique_ptr<FRunnableThread> RenderingThread;
    std::unique_ptr<FRunnableThread> AsyncLoadingThread;

    BaseApplication::BaseApplication(GfxConfiguration &cfg) :
        m_Config(cfg)
    {
    }

    bool BaseApplication::Initialize()
    {
        m_bQuit = false;

        FClassRegistryBase::DeferredConstructFProperty();
        FCrc::Init();
        GRenderingThreadRunnable = new FRenderingThread;
        GAsyncLoadingThreadRunnable = new FAsyncLoadingThread;
        RenderingThread = FRunnableThread::Create(GRenderingThreadRunnable, "Rendering Thread");
        while (!RenderingThread->IsRunnableInitialized()) { }
        AsyncLoadingThread = FRunnableThread::Create(GAsyncLoadingThreadRunnable, "Async Loading Thread");
        while (!AsyncLoadingThread->IsRunnableInitialized()) { }
        GameViewportClient = std::make_unique<UGameViewportClient>();
        GameViewportClient->Init();
        GameViewportClient->BeginPlay();
        return true;
    }


    void BaseApplication::Finalize()
    {
        RenderingThread->Kill();
        AsyncLoadingThread->Kill();
    }

    void BaseApplication::Finalize_RenderThread()
    {
        GameViewportClient->World = nullptr;
        GetScene()->Release_RenderThread();
    }

    void BaseApplication::Tick(double DeltaTime)
    {
        ENQUEUE_RENDER_COMMAND(BaseApplication_BeginFrame)(
            [this](RenderGraph&) 
            {
                FRenderingThread::RenderingThread->NotifyStartOfFrame();
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
            [this, &fence](RenderGraph& Graph) 
            {
                this->Tick_RenderThread();
                FRenderingThread::RenderingThread->NotifyEndOfFrame();
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
