#include <algorithm>


#include "DynamicRHI.h"
#include "Scene.h"
#include "BaseApplication.h"
#include "DefferedShadingSceneRenderer.h"
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

    FScene::FScene()
        : SkyAtmosphere(nullptr)
        , World(nullptr)
    {

    }

    void FScene::AddSkyAtmosphere(FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy)
    {
        FScene *Scene = this;

        ENQUEUE_RENDER_COMMAND(FScene_AddSkyAtmosphere)(
            [Scene, SkyAtmosphereSceneProxy](FDynamicRHI*)
            {
                Scene->SkyAtmosphereStack.emplace_back(SkyAtmosphereSceneProxy);
                Scene->SkyAtmosphere = SkyAtmosphereSceneProxy;
            });

    }

    void FScene::RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy)
    {
        FScene *Scene = this;

        ENQUEUE_RENDER_COMMAND(FScene_AddSkyAtmosphere)(
            [Scene, SkyAtmosphereSceneProxy](FDynamicRHI*)
            {
                safe_erase(Scene->SkyAtmosphereStack, SkyAtmosphereSceneProxy);
                if (!Scene->SkyAtmosphereStack.empty())
                    Scene->SkyAtmosphere = Scene->SkyAtmosphereStack.back().get();
                else
                    Scene->SkyAtmosphere = nullptr;
            });

    }

    void FScene::AddCamera(UCameraComponent *InCamera)
    {
        FCameraSceneProxy *CameraSceneProxy = InCamera->CreateSceneProxy();
        if (CameraSceneProxy == nullptr)
            return;

        FViewSceneInfo *ViewSceneInfo = new FViewSceneInfo(CameraSceneProxy, InCamera, this);
        CameraSceneProxy->ViewSceneInfo = ViewSceneInfo;

        FScene *Scene = this;
        ENQUEUE_RENDER_COMMAND(AddCamera)(
            [Scene, ViewSceneInfo] (FDynamicRHI *DynamicRHI) 
            {
                Scene->AddViewSceneInfo(ViewSceneInfo);
                ViewSceneInfo->SceneProxy->UpdateUniformBuffer();
            });

    }

    void FScene::RemoveCamera(UCameraComponent *InCamera)
    {
        FCameraSceneProxy* CameraSceneProxy = InCamera->SceneProxy;

        if (CameraSceneProxy)
        {
            FViewSceneInfo* ViewSceneInfo = CameraSceneProxy->GetViewSceneInfo();
            InCamera->SceneProxy = nullptr;
            
            FScene *Scene = this;
            ENQUEUE_RENDER_COMMAND(RemoveCamera)(
                [Scene, ViewSceneInfo] (FDynamicRHI *DynamicRHI) 
                {
                    Scene->RemoveViewSceneInfo(ViewSceneInfo);
                });
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
        ENQUEUE_RENDER_COMMAND(AddLight)(
            [Scene, LightSceneInfo] (FDynamicRHI *DynamicRHI) 
            {
                Scene->AddLightSceneInfo(LightSceneInfo);
                LightSceneInfo->SceneProxy->UpdateUniformBuffer();
            });
    }

    void FScene::RemoveLight(ULightComponent *InLight)
    {
        FLightSceneProxy* LightSceneProxy = InLight->SceneProxy;

        if (LightSceneProxy)
        {
            FLightSceneInfo* LightSceneInfo = LightSceneProxy->GetLightSceneInfo();
            InLight->SceneProxy = nullptr;
            
            FScene *Scene = this;
            ENQUEUE_RENDER_COMMAND(RemoveLight)(
                [Scene, LightSceneInfo] (FDynamicRHI *DynamicRHI) 
                {
                    Scene->RemoveLightSceneInfo(LightSceneInfo);
                });
        }
    }

    void FScene::AddPrimitive(UPrimitiveComponent *InPrimitive)
    {
        FPrimitiveSceneProxy *PrimitiveSceneProxy = InPrimitive->CreateSceneProxy();
        if (PrimitiveSceneProxy == nullptr)
            return;
            
        InPrimitive->SceneProxy = PrimitiveSceneProxy;
        FPrimitiveSceneInfo *PrimitiveSceneInfo = new FPrimitiveSceneInfo(PrimitiveSceneProxy, InPrimitive, this);
        PrimitiveSceneProxy->PrimitiveSceneInfo = PrimitiveSceneInfo;

        glm::mat4 RenderMatrix = InPrimitive->GetRenderMatrix();
        FBoundingBox Bounds = InPrimitive->GetBounds();

        FScene *Scene = this;
        ENQUEUE_RENDER_COMMAND(AddPrimitive)(
            [Scene, PrimitiveSceneProxy, PrimitiveSceneInfo, RenderMatrix, Bounds] (FDynamicRHI *DynamicRHI) 
            {
                PrimitiveSceneProxy->CreateRenderThreadResources();
                PrimitiveSceneProxy->SetTransform(RenderMatrix, Bounds);
                Scene->AddPrimitiveSceneInfo(PrimitiveSceneInfo);
            });
    }

    void FScene::RemovePrimitive(UPrimitiveComponent *InPrimitive)
    {
        FPrimitiveSceneProxy* PrimitiveSceneProxy = InPrimitive->SceneProxy;

        if (PrimitiveSceneProxy)
        {
            FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveSceneProxy->GetPrimitiveSceneInfo();
            InPrimitive->SceneProxy = nullptr;

            FScene *Scene = this;
            ENQUEUE_RENDER_COMMAND(AddPrimitive)(
                [Scene, PrimitiveSceneProxy, PrimitiveSceneInfo] (FDynamicRHI *DynamicRHI) 
                {
                    PrimitiveSceneInfo->SceneProxy->DestroyRenderThreadResources();
                    Scene->RemovePrimitiveSceneInfo(PrimitiveSceneInfo);
                });
        }
    }

    // void FScene::AddSceneCapture(USceneCaptureComponent *InCamera)
    // {
    //     FSceneCaptureInfo *SceneCaptureInfo = new FSceneCaptureInfo;

    //     FScene *Scene = this;
    //     ENQUEUE_RENDER_COMMAND(AddCamera)(
    //         [Scene, SceneCaptureInfo] (FDynamicRHI *DynamicRHI) 
    //         {
    //             Scene->AddViewSceneInfo(SceneCaptureInfo);
    //             ViewSceneInfo->SceneProxy->UpdateUniformBuffer();
    //         });

    // }

    // void FScene::RemoveSceneCapture(USceneCaptureComponent *InCamera)
    // {
    //     FCameraSceneProxy* CameraSceneProxy = InCamera->SceneProxy;

    //     if (CameraSceneProxy)
    //     {
    //         FViewSceneInfo* ViewSceneInfo = CameraSceneProxy->GetViewSceneInfo();
    //         InCamera->SceneProxy = nullptr;
            
    //         FScene *Scene = this;
    //         ENQUEUE_RENDER_COMMAND(RemoveCamera)(
    //             [Scene, ViewSceneInfo] (FDynamicRHI *DynamicRHI) 
    //             {
    //                 Scene->RemoveViewSceneInfo(ViewSceneInfo);
    //             });
    //     }
    // }

    void FScene::UpdateRenderInfos()
    {
        UpdateViewInfos();
        UpdatePrimitiveInfos();
        UpdateLightInfos();
    }

    void FScene::AddPrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        AddedPrimitiveSceneInfos.emplace(InPrimitiveInfo);
        InPrimitiveInfo->OcclusionQuery = FDynamicRHI::GetDynamicRHI()->RHICreateRenderQuery();
    }

    void FScene::RemovePrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        safe_erase(AddedPrimitiveSceneInfos, InPrimitiveInfo);
    }

    void FScene::AddLightSceneInfo(FLightSceneInfo *InLightInfo)
    {
        AddedLightSceneInfos.emplace(InLightInfo);
        GetAddLightDelegate().Broadcast(InLightInfo);
    }

    void FScene::RemoveLightSceneInfo(FLightSceneInfo *InLightInfo)
    {
        safe_erase(AddedLightSceneInfos, InLightInfo);
        GetRemoveLightDelegate().Broadcast(InLightInfo);
    }

    void FScene::AddViewSceneInfo(FViewSceneInfo *InCameraInfo)
    {
        AddedViewSceneInfos.emplace(InCameraInfo);
        GetAddViewDelegate().Broadcast(InCameraInfo);
    }

    void FScene::RemoveViewSceneInfo(FViewSceneInfo *InCameraInfo)
    {
        safe_erase(AddedViewSceneInfos, InCameraInfo);
        GetRemoveViewDelegate().Broadcast(InCameraInfo);
    }

    void FScene::UpdateViewInfos()
    {
        for (auto &&CameraInfo : AddedViewSceneInfos)
        {
            if (CameraInfo->bNeedsUniformBufferUpdate)
            {
                CameraInfo->SceneProxy->UpdateUniformBuffer();
                CameraInfo->SetNeedsUniformBufferUpdate(false);
            }
            if (CameraInfo->bNeedsFramebufferUpdate)
            {
                GetResizeViewDelegate().Broadcast(CameraInfo.get());
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
    
    void FScene::UpdateLightInfos()
    {
        for (auto &&LightInfo : AddedLightSceneInfos)
        {
            if (LightInfo->bNeedsUniformBufferUpdate)
                LightInfo->SceneProxy->UpdateUniformBuffer();
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
