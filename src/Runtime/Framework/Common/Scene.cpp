#include <algorithm>


#include "DynamicRHI.h"
#include "Scene.h"
#include "Common/BaseApplication.h"
// #include "Common/Components/PrimitiveSceneProxy.h"
// #include "Common/Components/LightSceneProxy.h"

namespace nilou {

    template<typename T>
    auto safe_erase(std::set<std::unique_ptr<T>>& set, T* ptr)
    {
        std::unique_ptr<T> stale_ptr{ptr};

        auto release = [](std::unique_ptr<T>* p) { (void*)p->release(); };
        std::unique_ptr<std::unique_ptr<T>, decltype(release)> release_helper{&stale_ptr, release};

        return set.erase(stale_ptr);
    }

    /**
    * if there's not release_helper and vec.erase throws an exception, this release() will not be called, 
    * thus causing some problems.
    */
    template<typename T>
    auto safe_erase(std::vector<std::unique_ptr<T>>& vec, T* ptr)
    {
        std::unique_ptr<T> stale_ptr{ptr};

        auto release = [](std::unique_ptr<T>* p) { (void*)p->release(); };
        std::unique_ptr<std::unique_ptr<T>, decltype(release)> release_helper{&stale_ptr, release};
        auto iter = std::find(vec.begin(), vec.end(), stale_ptr);
        return vec.erase(iter);
    }

    void FScene::AddSkyAtmosphere(FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy)
    {
        FScene *Scene = this;

        {
			Scene->SkyAtmosphereStack.emplace_back(SkyAtmosphereSceneProxy);
            Scene->SkyAtmosphere = SkyAtmosphereSceneProxy;
        }

    }

    void FScene::RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy)
    {
        FScene *Scene = this;

        {
            safe_erase(Scene->SkyAtmosphereStack, SkyAtmosphereSceneProxy);
            // auto iter = std::find(Scene->SkyAtmosphereStack.begin(), Scene->SkyAtmosphereStack.end(), SkyAtmosphereSceneProxy);
            // Scene->SkyAtmosphereStack.erase(iter);
            if (!SkyAtmosphereStack.empty())
                Scene->SkyAtmosphere = SkyAtmosphereStack.back().get();
            else
                Scene->SkyAtmosphere = nullptr;
        }

    }

    void FScene::AddCamera(UCameraComponent *InCamera)
    {
        FCameraSceneProxy *CameraSceneProxy = InCamera->CreateSceneProxy();
        if (CameraSceneProxy == nullptr)
            return;

        FCameraSceneInfo *CameraSceneInfo = new FCameraSceneInfo(CameraSceneProxy, InCamera, this);
        CameraSceneProxy->CameraSceneInfo = CameraSceneInfo;

        FScene *Scene = this;
        {
            Scene->AddCameraSceneInfo(CameraSceneInfo);
            CameraSceneInfo->SceneProxy->UpdateUniformBuffer();
        }
    }

    void FScene::RemoveCamera(UCameraComponent *InCamera)
    {
        FCameraSceneProxy* CameraSceneProxy = InCamera->SceneProxy;

        if (CameraSceneProxy)
        {
            FCameraSceneInfo* LightSceneInfo = CameraSceneProxy->GetCameraSceneInfo();
            InCamera->SceneProxy = nullptr;
            
            FScene *Scene = this;
            {
                Scene->RemoveCameraSceneInfo(LightSceneInfo);
            }
        }
    }

    void FScene::AddLight(ULightComponent *InLight)
    {
        FLightSceneProxy *LightSceneProxy = InLight->CreateSceneProxy();
        if (LightSceneProxy == nullptr)
            return;

        FLightSceneInfo *LightSceneInfo = new FLightSceneInfo(LightSceneProxy, InLight, this);
        LightSceneProxy->LightSceneInfo = LightSceneInfo;

        FScene *Scene = this;
        {
            Scene->AddLightSceneInfo(LightSceneInfo);
            LightSceneInfo->SceneProxy->UpdateUniformBuffer();
        }
    }

    void FScene::RemoveLight(ULightComponent *InLight)
    {
        FLightSceneProxy* LightSceneProxy = InLight->SceneProxy;

        if (LightSceneProxy)
        {
            FLightSceneInfo* LightSceneInfo = LightSceneProxy->GetLightSceneInfo();
            InLight->SceneProxy = nullptr;
            
            FScene *Scene = this;
            {
                Scene->RemoveLightSceneInfo(LightSceneInfo);
            }
        }
    }

    void FScene::AddPrimitive(UPrimitiveComponent *InPrimitive)
    {
        FPrimitiveSceneProxy *PrimitiveSceneProxy = InPrimitive->CreateSceneProxy();
        if (PrimitiveSceneProxy == nullptr)
            return;

        FPrimitiveSceneInfo *PrimitiveSceneInfo = new FPrimitiveSceneInfo(PrimitiveSceneProxy, InPrimitive, this);
        PrimitiveSceneProxy->PrimitiveSceneInfo = PrimitiveSceneInfo;

        glm::mat4 RenderMatrix = InPrimitive->GetRenderMatrix();
        FBoundingBox Bounds = InPrimitive->GetBounds();

        FScene *Scene = this;
        {
            PrimitiveSceneProxy->SetTransform(RenderMatrix, Bounds);
            PrimitiveSceneProxy->CreateRenderThreadResources();
            Scene->AddPrimitiveSceneInfo(PrimitiveSceneInfo);
        }
    }

    void FScene::RemovePrimitive(UPrimitiveComponent *InPrimitive)
    {
        FPrimitiveSceneProxy* PrimitiveSceneProxy = InPrimitive->SceneProxy;

        if (PrimitiveSceneProxy)
        {
            FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveSceneProxy->GetPrimitiveSceneInfo();
            InPrimitive->SceneProxy = nullptr;

            FScene *Scene = this;
            {
                PrimitiveSceneInfo->SceneProxy->DestroyRenderThreadResources();
                Scene->RemovePrimitiveSceneInfo(PrimitiveSceneInfo);
            }
        }
    }

    void FScene::AddPrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        AddedPrimitiveSceneInfos.emplace(InPrimitiveInfo);
    }

    void FScene::RemovePrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        safe_erase(AddedPrimitiveSceneInfos, InPrimitiveInfo);
    }

    void FScene::AddLightSceneInfo(FLightSceneInfo *InLightInfo)
    {
        AddedLightSceneInfos.emplace(InLightInfo);
    }

    void FScene::RemoveLightSceneInfo(FLightSceneInfo *InLightInfo)
    {
        safe_erase(AddedLightSceneInfos, InLightInfo);
    }

    void FScene::AddCameraSceneInfo(FCameraSceneInfo *InCameraInfo)
    {
        AddedCameraSceneInfos.emplace_back(InCameraInfo);
    }

    void FScene::RemoveCameraSceneInfo(FCameraSceneInfo *InCameraInfo)
    {
        safe_erase(AddedCameraSceneInfos, InCameraInfo);
        // auto iter = std::find(AddedCameraSceneInfos.begin(), AddedCameraSceneInfos.end(), InCameraInfo);
        // AddedCameraSceneInfos.erase(iter);
        // delete InCameraInfo;
    }

    RHIFramebufferRef CreateSceneTextures(const ivec2 &ScreenResolution, FSceneTextures &OutSceneTextures)
    {
        RHIFramebufferRef FrameBuffer = GDynamicRHI->RHICreateFramebuffer();
        OutSceneTextures.BaseColor = GDynamicRHI->RHICreateTexture2D(
            "BaseColor", EPixelFormat::PF_R32G32B32A32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.WorldSpacePosition = GDynamicRHI->RHICreateTexture2D(
            "WorldSpacePosition", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.WorldSpaceNormal = GDynamicRHI->RHICreateTexture2D(
            "WorldSpaceNormal", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.MetallicRoughness = GDynamicRHI->RHICreateTexture2D(
            "MetallicRoughness", EPixelFormat::PF_R32G32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.Emissive = GDynamicRHI->RHICreateTexture2D(
            "Emissive", EPixelFormat::PF_R32G32B32F, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        OutSceneTextures.DepthStencil = GDynamicRHI->RHICreateTexture2D(
            "DepthStencil", EPixelFormat::PF_D24S8, 1, 
            ScreenResolution.x, ScreenResolution.y, nullptr);

        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment0, OutSceneTextures.BaseColor);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment1, OutSceneTextures.WorldSpacePosition);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment2, OutSceneTextures.WorldSpaceNormal);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment3, OutSceneTextures.MetallicRoughness);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Color_Attachment4, OutSceneTextures.Emissive);
        FrameBuffer->AddAttachment(EFramebufferAttachment::FA_Depth_Stencil_Attachment, OutSceneTextures.DepthStencil);
        return FrameBuffer;
    }

    void FScene::UpdateViewInfos()
    {
        for (int ViewIndex = 0; ViewIndex < AddedCameraSceneInfos.size(); ViewIndex++)
        {
            FCameraSceneInfo *CameraInfo = AddedCameraSceneInfos[ViewIndex].get();
            if (CameraInfo->bNeedsUniformBufferUpdate)
            {
                CameraInfo->SceneProxy->UpdateUniformBuffer();
                CameraInfo->SetNeedsUniformBufferUpdate(false);
            }
            if (CameraInfo->bNeedsFramebufferUpdate)
            {
                CameraInfo->FrameBuffer = CreateSceneTextures(CameraInfo->SceneProxy->ScreenResolution, CameraInfo->SceneTextures);
                CameraInfo->SetNeedsFramebufferUpdate(false);
            }
        }
    }

    void FScene::UpdatePrimitiveInfos()
    {
        for (auto &&PrimitiveInfo : AddedPrimitiveSceneInfos)
        {
            if (PrimitiveInfo->bNeedsUniformBufferUpdate)
                PrimitiveInfo->SceneProxy->UpdateUniformBuffer();
        }
    }
}






// nilou::Scene::Scene()
//     : SceneName(""), SceneGraph(new BaseSceneNode())
// {

// }

// nilou::Scene::Scene(const std::string &scene_name)
//     : SceneName(scene_name), SceneGraph(new BaseSceneNode())
// {

// }

// void nilou::Scene::AddObserver(std::shared_ptr<nilou::ObserverActor> observer)
// {
//     Observer = observer;
//     SceneGraph->AppendChild(observer->GetRootSceneNode());
// }

// void nilou::Scene::AddLight(std::shared_ptr<nilou::SceneLightNode> light)
// {
//     SceneGraph->AppendChild(light);
//     LightNodes.push_back(light);
//     Lights.push_back(light->GetSceneObjectRef());
// }

// void nilou::Scene::AddSkybox(std::shared_ptr<nilou::SceneObjectSkybox> skybox)
// {
//     Skybox = skybox;
// }

// void nilou::Scene::AddOceanSurface(std::shared_ptr<SceneObjectOceanSurface> ocean_surface)
// {
//     OceanSurface = ocean_surface;
// }

// void nilou::Scene::AddSeabedSurface(std::shared_ptr<SceneObjectTerrainSurface> seabed_surface)
// {
//     SeabedSurface = seabed_surface;
// }