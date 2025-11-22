#include "RenderingThread.h"
#include "RenderGraph.h"
#include "RenderGraphResourcePool.h"
#include "ShaderCompiler.h"
#include "VulkanDynamicRHI.h"

namespace nilou {

    std::thread::id GRenderThreadId;
    uint32 GFrameNumberRenderThread = 0;
    
    FRenderingThread *FRenderingThread::RenderingThread = nullptr;
    std::mutex FRenderingThread::mutex;
    std::queue<EnqueueUniqueRenderCommandType> FRenderingThread::RenderCommands;
    std::function<void()> FRenderingThread::PreRenderThreadInitDelegate;

    void EnqueueUniqueRenderCommandType::DoTask()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        lambda(Graph);
    }

    bool FRenderingThread::Init()
    {
        RenderingThread = this;
        GRenderThreadId = std::this_thread::get_id();
        // PreRenderThreadInitDelegate();
        GDynamicRHI = new FVulkanDynamicRHI(GGfxConfig);
        GDynamicRHI->Initialize();
        GraphRecording = std::make_unique<RenderGraph>();
        return true;
    }

    uint32 FRenderingThread::Run()
    {
        while (!bShouldExit)
        {
            EnqueueUniqueRenderCommandType RenderCommand;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (!RenderCommands.empty())
                {
                    RenderCommand = std::move(RenderCommands.front());
                    RenderCommands.pop();
                }
            }
            if (RenderCommand.IsValid())
            {
                RenderCommand.DoTask();
            }
        }
        return 0;
    }

    void FRenderingThread::NotifyStartOfFrame()
    {
        if (!GraphRecording)
        {
            GraphRecording = std::make_unique<RenderGraph>();
        }
        GraphRecording->BeginFrame();
    }

    void FRenderingThread::NotifyEndOfFrame()
    {
        GraphExecuting = std::move(GraphRecording);
        GraphExecuting->Execute();
        GraphExecuting->EndFrame();
        GRenderGraphBufferPool.TickPoolElements();
        GRenderGraphTexturePool.TickPoolElements();
        GFrameNumberRenderThread++;
    }

    void FRenderingThread::Stop()
    {
        bShouldExit = true;
    }

    void FRenderingThread::Exit()
    {
        // Some release works may be done in the for loop.
        while (!RenderCommands.empty())
        {
            EnqueueUniqueRenderCommandType RenderCommand;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (!RenderCommands.empty())
                {
                    RenderCommand = std::move(RenderCommands.front());
                    RenderCommands.pop();
                }
            }
            if (RenderCommand.IsValid())
            {
                RenderCommand.DoTask();
            }
        }
        std::vector<FRenderResource*>& ResourceList = FRenderResource::GetResourceList();
        for (int i = 0; i < ResourceList.size(); i++)
        {
            FRenderResource* Resource = ResourceList[i];
            if (Resource)
                Resource->ReleaseResource();
        }
        // FSceneRenderer::ShadowMapResourcesPool.ReleaseAll();
        // FSceneRenderer::SceneTexturesPool.ReleaseAll();
        // GetAppication()->Finalize_RenderThread();
        FDynamicRHI::Get()->Finalize();
    }

    FRenderingThread::~FRenderingThread()
    {
        
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
