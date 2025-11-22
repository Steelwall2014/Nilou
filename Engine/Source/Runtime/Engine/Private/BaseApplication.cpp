#include "BaseApplication.h"
#include "NObject/NilouType.h"
#include "RHICommandList.h"
#include "Misc/Paths.h"
#include "Misc/Crc.h"
#include "Serialization/AsyncLoading.h"
#include "ShaderCompiler.h"

namespace nilou {

    std::atomic<bool> BaseApplication::m_bQuit = false;
    
    FRunnable* GRenderingThreadRunnable = nullptr;
    FRunnable* GAsyncLoadingThreadRunnable = nullptr;
    std::unique_ptr<FRunnableThread> RenderingThread;
    std::unique_ptr<FRunnableThread> AsyncLoadingThread;

    uint32 GFrameNumber = 0;

    BaseApplication::BaseApplication()
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
        ENQUEUE_RENDER_COMMAND(BaseApplication_Initialize)(
            [this](RenderGraph&) 
            {
                this->Initialize_RenderThread();
            });
        GameViewportClient->BeginPlay();
        return true;
    }

    bool BaseApplication::Initialize_RenderThread()
    {
        FShaderCompiler::CompileGlobalShaders();
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
        ProcessInput();
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
        GFrameNumber++;
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
        return GGfxConfig;
    }

    void BaseApplication::SetWindowWidth(int width)
    {
        GGfxConfig.screenWidth = width;
    }

    void BaseApplication::SetWindowHeight(int height)
    {
        GGfxConfig.screenHeight = height;
    }

    float BaseApplication::GetTimeSinceStart()
    {
        return accumTime;
    }

    bool BaseApplication::IsCursorEnabled()
    {
        return CursorEnabled;
    }

    BaseApplication *GApplication = nullptr;

}
