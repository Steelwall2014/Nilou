#include <glad.h>
#include <GLFW/glfw3.h>

#include "DeferredShadingSceneRenderer.h"
// #include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "BaseApplication.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "RenderGraph.h"

namespace nilou {

    std::thread::id GRenderThreadId;
    
    FRenderingThread *FRenderingThread::RenderingThread = nullptr;

    void EnqueueUniqueRenderCommandType::DoTask()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        lambda(Graph);
    }

    bool FRenderingThread::Init()
    {
        RenderingThread = this;
        GRenderThreadId = std::this_thread::get_id();
        FDynamicRHI::CreateDynamicRHI_RenderThread(GetAppication()->GetConfiguration());
        GetAppication()->Initialize_RenderThread();
        FDynamicRHI::GetDynamicRHI()->Initialize();
        // AddShaderSourceDirectoryMapping("/Shaders", FPath::ShaderDir().generic_string());
        FShaderCompiler::CompileGlobalShaders();
        GetContentManager()->Init();
        return true;
    }

    uint32 FRenderingThread::FrameCount = 0;

    uint32 FRenderingThread::Run()
    {
        while (!GetAppication()->ShouldRenderingThreadExit())
        {
            int size = RenderCommands.size();
            for (int i = 0; i < size; i++)
            {
                EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
                std::unique_lock<std::mutex> lock(mutex);
                RenderCommands.pop();
                lock.unlock();
                RenderCommand.DoTask();
            }
        }
        return 0;
    }

    void FRenderingThread::NotifyStartOfFrame()
    {
        FRenderingThread* _this = RenderingThread;
        if (_this->GraphExucuting)
        {
            _this->GraphExucuting->Compile();
            _this->GraphExucuting->Execute();
        }
    }

    void FRenderingThread::NotifyEndOfFrame()
    {
        FrameCount++;
        FRenderingThread* _this = RenderingThread;
        if (_this->GraphExucuting)
        {
            delete _this->GraphExucuting;
            _this->GraphExucuting = nullptr;
        }
        _this->GraphExucuting = _this->GraphRecording;
        _this->GraphRecording = new RenderGraph();
    }

    void FRenderingThread::Exit()
    {
        // Some release works may be done in the for loop.
        for (int i = 0; i < RenderCommands.size(); i++)
        {
            EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
            std::unique_lock<std::mutex> lock(mutex);
            RenderCommands.pop();
            lock.unlock();
            RenderCommand.DoTask();
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
        GetAppication()->Finalize_RenderThread();
        FDynamicRHI::GetDynamicRHI()->Finalize();
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
