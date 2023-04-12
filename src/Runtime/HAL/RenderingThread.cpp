#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefferedShadingSceneRenderer.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "BaseApplication.h"
#include "Common/ContentManager.h"
#include "Material.h"

namespace nilou {

    std::thread::id GRenderThreadId;
    
    FRenderingThread *FRenderingThread::RenderingThread = nullptr;

    bool FRenderingThread::Init()
    {
        RenderingThread = this;
        GRenderThreadId = std::this_thread::get_id();
        GetAppication()->Initialize_RenderThread();
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        FDynamicRHI::CreateDynamicRHI_RenderThread();
        FDynamicRHI::GetDynamicRHI()->Initialize();
        AddShaderSourceDirectoryMapping("/Shaders", FPath::ShaderDir().generic_string());
        FShaderCompiler::CompileGlobalShaders(FDynamicRHI::GetDynamicRHI());
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
            FRenderingThread::FrameCount++;
        }
        return 0;
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
        GetAppication()->Finalize_RenderThread();
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
