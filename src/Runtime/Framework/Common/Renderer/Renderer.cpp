#include "Renderer.h"
#include "Common/BaseApplication.h"
#include "Common/Scene.h"
#include "Common/World.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "DefferedShadingSceneRenderer.h"
#include "ShaderCompiler.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    int FRendererModule::StartupModule()
    {
        GDynamicRHI = new FOpenGLDynamicRHI;
        GDynamicRHI->Initialize();
        AddShaderSourceDirectoryMapping("/Shaders", "D:\\Nilou\\Assets\\Shaders");
        FShaderCompiler::CompileGlobalShaders();
        Renderer = (FDefferedShadingSceneRenderer *)FSceneRenderer::CreateSceneRenderer(GetAppication()->GetScene());
        return 0;
    }

    void FRendererModule::ShutdownModule()
    {

    }

    void FRendererModule::Draw(FScene *Scene)
    {
        if (Scene)
        {
            UWorld *World = Scene->World;
            if (World)
                World->SendAllEndOfFrameUpdates();
        }

        {
            if (Scene)
            {
                Scene->UpdateRenderInfos();
            }
            Draw_RenderThread(Scene);
        }
    }

    void FRendererModule::Draw_RenderThread(FScene *Scene)
    {
        Renderer->Render();
    }

    IMPLEMENT_MODULE(FRendererModule)

}