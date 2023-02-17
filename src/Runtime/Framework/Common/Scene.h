#pragma once
#include <memory>
#include <set>
#include <unordered_map>
#include <queue>

#include "Common/Components/CameraComponent.h"
#include "Common/Components/LightComponent.h"
#include "Common/Components/PrimitiveComponent.h"
#include "Common/Components/SkyAtmosphereComponent.h"
#include "Common/BatchedLine.h"
#include "ViewElementPDI.h"
// #include "SceneNode.h"
// #include "SceneObject.h"
// #include "Common/Actor/ObserverActor.h"

namespace nilou {
    class USceneComponent;
    class FPrimitiveSceneInfo;

    class FPrimitiveSceneInfo
    {
    public:
        FPrimitiveSceneInfo(FPrimitiveSceneProxy *InSceneProxy, UPrimitiveComponent *InPrimitive, FScene *InScene)
            : Primitive(InPrimitive)
            , Scene(InScene)
            , SceneProxy(InSceneProxy)
        {

        }

        void SetNeedsUniformBufferUpdate(bool bInNeedsUniformBufferUpdate)
        {
            bNeedsUniformBufferUpdate = bInNeedsUniformBufferUpdate;
        }

        // FUniformBuffer *GetUniformBuffer() { return PrimitiveShaderUniformBuffer.get(); }
    
        // void UpdateUniformBuffer() { PrimitiveShaderUniformBuffer->UpdateUniformBuffer(); }

        FScene *Scene;
        UPrimitiveComponent *Primitive;
        FPrimitiveSceneProxy *SceneProxy;
        bool bNeedsUniformBufferUpdate = false;
    };

    class FLightSceneInfo
    {
    public:
        FLightSceneInfo(FLightSceneProxy *InSceneProxy, ULightComponent *InLight, FScene *InScene)
            : Light(InLight)
            , Scene(InScene)
            , SceneProxy(InSceneProxy)
        {

        }

        void SetNeedsUniformBufferUpdate(bool bInNeedsUniformBufferUpdate)
        {
            bNeedsUniformBufferUpdate = bInNeedsUniformBufferUpdate;
        }

        FScene *Scene;
        ULightComponent *Light;
        FLightSceneProxy *SceneProxy;
        std::vector<FSceneLightView> LightViews;
        std::vector<RHIFramebufferRef> ShadowMapRenderTarget;
        std::vector<std::vector<RHITexture2DRef>> ShadowMapDepthTextures;
        int LightUniformBufferIndex;
        bool bNeedsUniformBufferUpdate = false;
    };

    class FViewSceneInfo
    {
    public:
        friend class FScene;
        friend class FDefferedShadingSceneRenderer;
        FViewSceneInfo(FCameraSceneProxy *InSceneProxy, UCameraComponent *InCamera, FScene *InScene)
            : Camera(InCamera)
            , Scene(InScene)
            , SceneProxy(InSceneProxy)
        {
            PDI = std::make_shared<FViewElementPDI>();
        }

        void SetNeedsUniformBufferUpdate(bool bInNeedsUniformBufferUpdate)
        {
            bNeedsUniformBufferUpdate = bInNeedsUniformBufferUpdate;
        }

        void SetNeedsFramebufferUpdate(bool bInNeedsFramebufferUpdate)
        {
            bNeedsFramebufferUpdate = bInNeedsFramebufferUpdate;
        }

        ivec2 GetResolution() const { return SceneProxy->GetSceneView().ScreenResolution; }

        FScene *Scene;
        UCameraComponent *Camera;
        FCameraSceneProxy *SceneProxy;
        std::shared_ptr<FViewElementPDI> PDI;
    private:
        bool bNeedsUniformBufferUpdate = false;
        bool bNeedsFramebufferUpdate = false;
    };

    class FScene
    {
    public:

        FScene();

        void AddSkyAtmosphere(FSkyAtmosphereSceneProxy *InSkyAtmosphereProxy);
        void RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy *InSkyAtmosphereProxy);

        void AddCamera(UCameraComponent *InCamera);
        void RemoveCamera(UCameraComponent *InCamera);

        void AddLight(ULightComponent *InLight);
        void RemoveLight(ULightComponent *InLight);

        void AddPrimitive(UPrimitiveComponent *InPrimitive);
        void RemovePrimitive(UPrimitiveComponent *InPrimitive);

        void UpdateRenderInfos();


        // void UpdatePrimitiveTransform(UPrimitiveComponent *InPrimitive);

        // void UpdateAllPrimitiveSceneInfos();

        // inline FLightSceneInfo *GetFirstDirectionalLight() const { return DirectionalLightQueue.empty() ? nullptr : DirectionalLightQueue.front(); }

        std::set<std::unique_ptr<FPrimitiveSceneInfo>> AddedPrimitiveSceneInfos;
        std::set<std::unique_ptr<FLightSceneInfo>> AddedLightSceneInfos;
        std::set<std::unique_ptr<FViewSceneInfo>> AddedViewSceneInfos;
        std::vector<std::unique_ptr<FSkyAtmosphereSceneProxy>> SkyAtmosphereStack;
        FSkyAtmosphereSceneProxy *SkyAtmosphere;
        class UWorld *World;

    protected:

        // std::list<FLightSceneInfo *> DirectionalLightQueue;

        /** 下面几个在ue里是RenderThread */
        void AddPrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);
        void RemovePrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);

        void AddLightSceneInfo(FLightSceneInfo *InLightInfo);
        void RemoveLightSceneInfo(FLightSceneInfo *InLightInfo);

        void AddViewSceneInfo(FViewSceneInfo *InCameraInfo);
        void RemoveViewSceneInfo(FViewSceneInfo *InCameraInfo);

        void UpdateViewInfos();
        void UpdatePrimitiveInfos();
        void UpdateLightInfos();
    };
}