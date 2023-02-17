#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefferedShadingSceneRenderer.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "Common/BaseApplication.h"

namespace nilou {

    std::thread::id GRenderThreadId;
    

    bool FRenderingThread::Init()
    {
        GRenderThreadId = std::this_thread::get_id();
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        GetAppication()->Initialize_RenderThread();
        
        GDynamicRHI = new FOpenGLDynamicRHI;
        GDynamicRHI->Initialize();
        AddShaderSourceDirectoryMapping("/Shaders", "D:\\Nilou\\Assets\\Shaders");
        FShaderCompiler::CompileGlobalShaders();
        Renderer = (FDefferedShadingSceneRenderer *)FSceneRenderer::CreateSceneRenderer(GetAppication()->GetScene());
        return true;
    }

    uint32 FRenderingThread::Run()
    {
        while (!GetAppication()->IsQuit())
        {
            std::cout << IsInRenderingThread();
        }
        return 0;
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
