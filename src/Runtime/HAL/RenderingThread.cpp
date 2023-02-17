#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefferedShadingSceneRenderer.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "Common/BaseApplication.h"

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
        AddShaderSourceDirectoryMapping("/Shaders", "D:\\Nilou\\Assets\\Shaders");
        FShaderCompiler::CompileGlobalShaders(FDynamicRHI::GetDynamicRHI());
        Renderer = (FDefferedShadingSceneRenderer *)FSceneRenderer::CreateSceneRenderer(GetAppication()->GetScene());
        return true;
    }

    uint32 FRenderingThread::Run()
    {
        while (!GetAppication()->IsQuit())
        {
            int size = RenderCommands.size();
            for (int i = 0; i < size; i++)
            {
                EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
                mutex.lock();
                RenderCommands.pop();
                mutex.unlock();
                RenderCommand.DoTask();
            }
            GetAppication()->Tick_RenderThread();
        }
        return 0;
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
