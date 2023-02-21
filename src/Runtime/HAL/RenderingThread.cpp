#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "DefferedShadingSceneRenderer.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "RenderingThread.h"
#include "Common/BaseApplication.h"
#include "Common/ContentManager.h"
#include "Common/FrameSynchronizer.h"
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
        
        std::shared_ptr<FMaterial> Cesium3DTilesMaterial = std::make_shared<FMaterial>("Cesium3DTilesMaterial");
        FContentManager::GetContentManager().AddGlobalMaterial(Cesium3DTilesMaterial->GetMaterialName(), Cesium3DTilesMaterial);
        Cesium3DTilesMaterial->UpdateMaterialCode(
        R"(
            #include "../include/BasePassCommon.glsl"

            uniform sampler2D baseColorTexture;
            uniform sampler2D metallicRoughnessTexture;
            uniform sampler2D emissiveTexture;
            // uniform sampler2D normalTexture;

            layout (std140) uniform FCesium3DTilesMaterialBlock {
                vec4 baseColorFactor;
                vec3 emissveFactor;
                float metallicFactor;
                float roughnessFactor;
            };

            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                return texture(emissiveTexture, vs_out.TexCoords).rgb * emissveFactor;
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                // vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
                vec3 tangent_normal = vec3(0.5, 0.5, 1.0);
                tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
                return normalize(vs_out.TBN * tangent_normal);
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return texture(metallicRoughnessTexture, vs_out.TexCoords).g;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return texture(metallicRoughnessTexture, vs_out.TexCoords).b;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )");
        RHITextureParams texParams;
        texParams.Mag_Filter = ETextureFilters::TF_Nearest;
        texParams.Min_Filter = ETextureFilters::TF_Nearest;
        texParams.Wrap_S = ETextureWrapModes::TW_Clamp;
        texParams.Wrap_T = ETextureWrapModes::TW_Clamp;
        std::shared_ptr<FImage> NoColorImg = std::make_shared<FImage>();
        NoColorImg->Width = 1; NoColorImg->Height = 1; NoColorImg->Channel = 4; NoColorImg->data_size = 4;
        NoColorImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoColorImg->data = new uint8[4];
        NoColorImg->data[0] = 255; NoColorImg->data[1] = 255; NoColorImg->data[2] = 255; NoColorImg->data[3] = 255;
        std::shared_ptr<FTexture> NoColorTexture = std::make_shared<FTexture>("NoColorTexture", 1, NoColorImg);
        NoColorTexture->SetSamplerParams(texParams);
        FContentManager::GetContentManager().AddGlobalTexture("NoColorTexture", NoColorTexture);
        BeginInitResource(NoColorTexture.get());

        std::shared_ptr<FImage> NoMetallicRoughnessImg = std::make_shared<FImage>();
        NoMetallicRoughnessImg->Width = 1; NoMetallicRoughnessImg->Height = 1; NoMetallicRoughnessImg->Channel = 4; NoMetallicRoughnessImg->data_size = 4;
        NoMetallicRoughnessImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoMetallicRoughnessImg->data = new uint8[4];
        NoMetallicRoughnessImg->data[0] = 0; NoMetallicRoughnessImg->data[1] = 255; NoMetallicRoughnessImg->data[2] = 255; NoMetallicRoughnessImg->data[3] = 255;
        std::shared_ptr<FTexture> NoMetallicRoughnessTexture = std::make_shared<FTexture>("NoMetallicRoughnessTexture", 1, NoMetallicRoughnessImg);
        FContentManager::GetContentManager().AddGlobalTexture("NoMetallicRoughnessTexture", NoMetallicRoughnessTexture);
        NoMetallicRoughnessTexture->SetSamplerParams(texParams);
        BeginInitResource(NoMetallicRoughnessTexture.get());

        std::shared_ptr<FImage> NoEmissiveImg = std::make_shared<FImage>();
        NoEmissiveImg->Width = 1; NoEmissiveImg->Height = 1; NoEmissiveImg->Channel = 4; NoEmissiveImg->data_size = 4;
        NoEmissiveImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoEmissiveImg->data = new uint8[4];
        NoEmissiveImg->data[0] = 0; NoEmissiveImg->data[1] = 0; NoEmissiveImg->data[2] = 0; NoEmissiveImg->data[3] = 255;
        std::shared_ptr<FTexture> NoEmissiveTexture = std::make_shared<FTexture>("NoEmissiveTexture", 1, NoEmissiveImg);
        FContentManager::GetContentManager().AddGlobalTexture("NoEmissiveTexture", NoEmissiveTexture);
        NoEmissiveTexture->SetSamplerParams(texParams);
        BeginInitResource(NoEmissiveTexture.get());

        std::shared_ptr<FImage> NoNormalImg = std::make_shared<FImage>();
        NoNormalImg->Width = 1; NoNormalImg->Height = 1; NoNormalImg->Channel = 4; NoNormalImg->data_size = 4;
        NoNormalImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoNormalImg->data = new uint8[4];
        NoNormalImg->data[0] = 127; NoNormalImg->data[1] = 127; NoNormalImg->data[2] = 255; NoNormalImg->data[3] = 255;
        std::shared_ptr<FTexture> NoNormalTexture = std::make_shared<FTexture>("NoNormalTexture", 1, NoNormalImg);
        FContentManager::GetContentManager().AddGlobalTexture("NoNormalTexture", NoNormalTexture);
        NoNormalTexture->SetSamplerParams(texParams);
        BeginInitResource(NoNormalTexture.get());

        Cesium3DTilesMaterial->SetParameterValue("baseColorTexture", NoColorTexture.get());
        Cesium3DTilesMaterial->SetParameterValue("metallicRoughnessTexture", NoMetallicRoughnessTexture.get());
        Cesium3DTilesMaterial->SetParameterValue("emissiveTexture", NoEmissiveTexture.get());
        Cesium3DTilesMaterial->SetParameterValue("normalTexture", NoNormalTexture.get());
        Cesium3DTilesMaterial->RasterizerState.CullMode = ERasterizerCullMode::CM_CCW;
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
        std::unique_lock<std::mutex> lock(FFrameSynchronizer::mutex);
        FFrameSynchronizer::cv.wait(lock, []{return FFrameSynchronizer::ShouldRenderingThreadLoopRun;});
        
        while (!GetAppication()->ShouldRenderingThreadExit())
        {
            int size = RenderCommands.size();
            // std::cout << size << std::endl;
            for (int i = 0; i < size; i++)
            {
                EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
                std::unique_lock<std::mutex> lock(mutex);
                RenderCommands.pop();
                lock.unlock();
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

        // Some release works may be done in the for loop.
        for (int i = 0; i < RenderCommands.size(); i++)
        {
            EnqueueUniqueRenderCommandType RenderCommand = RenderCommands.front();
            std::unique_lock<std::mutex> lock(mutex);
            RenderCommands.pop();
            lock.unlock();
            RenderCommand.DoTask();
        }
    }

    bool IsInRenderingThread()
    {
        return std::this_thread::get_id() == GRenderThreadId;
    }
}
