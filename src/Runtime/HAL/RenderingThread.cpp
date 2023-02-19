#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefferedShadingSceneRenderer.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "Common/BaseApplication.h"
#include "Common/ContentManager.h"
#include "Material.h"

namespace nilou {

    std::thread::id GRenderThreadId;
    
    FRenderingThread *FRenderingThread::RenderingThread = nullptr;

    void CreateMaterials()
    {
        std::shared_ptr<FMaterial> DefaultMaterial = std::make_shared<FMaterial>("DefaultMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(DefaultMaterial->GetMaterialName(), DefaultMaterial);
        DefaultMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0, 0, 0, 1);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");

        std::shared_ptr<FMaterial> ColoredMaterial = std::make_shared<FMaterial>("ColoredMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(ColoredMaterial->GetMaterialName(), ColoredMaterial);
        ColoredMaterial->RasterizerState.CullMode = CM_None;
        ColoredMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vs_out.Color;
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");

        std::shared_ptr<FMaterial> SkyAtmosphereMaterial = std::make_shared<FMaterial>("SkyAtmosphereMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(SkyAtmosphereMaterial->GetMaterialName(), SkyAtmosphereMaterial);
        SkyAtmosphereMaterial->RasterizerState.CullMode = CM_None;
        SkyAtmosphereMaterial->DepthStencilState.bEnableFrontFaceStencil = true;
        SkyAtmosphereMaterial->DepthStencilState.FrontFaceStencilTest = ECompareFunction::CF_Always;
        SkyAtmosphereMaterial->DepthStencilState.FrontFacePassStencilOp = EStencilOp::SO_Replace;
        SkyAtmosphereMaterial->DepthStencilState.bEnableBackFaceStencil = true;
        SkyAtmosphereMaterial->DepthStencilState.BackFaceStencilTest = ECompareFunction::CF_Always;
        SkyAtmosphereMaterial->DepthStencilState.BackFacePassStencilOp = EStencilOp::SO_Replace;
        SkyAtmosphereMaterial->StencilRefValue = 255;
        SkyAtmosphereMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");
        
        std::shared_ptr<FMaterial> WireframeMaterial = std::make_shared<FMaterial>("WireframeMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(WireframeMaterial->GetMaterialName(), WireframeMaterial);
        WireframeMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"
            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return vec4(0, 0, 0, 1);
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return vec3(0);
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                return normalize(vs_out.TBN * vec3(0, 0, 1));
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return 0.5;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return 0.5;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");
        WireframeMaterial->RasterizerState.FillMode = ERasterizerFillMode::FM_Wireframe;
    }

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
        CreateMaterials();
        return true;
    }

    uint32 FRenderingThread::Run()
    {
        while (!GetAppication()->ShouldRenderingThreadExit())
        {
            int size = RenderCommands.size();
            // std::cout << size << std::endl;
            for (int i = 0; i < size; i++)
            {
                EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
                mutex.lock();
                RenderCommands.pop();
                mutex.unlock();
                RenderCommand.DoTask();
            }
            GetAppication()->GetScene()->UpdateRenderInfos();
            Renderer->Render();
            GetAppication()->Tick_RenderThread();
        }
        return 0;
    }

    void FRenderingThread::Exit()
    {
        GetAppication()->Finalize_RenderThread();
        for (int i = 0; i < RenderCommands.size(); i++)
        {
            EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
            mutex.lock();
            RenderCommands.pop();
            mutex.unlock();
            RenderCommand.DoTask();
        }
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
