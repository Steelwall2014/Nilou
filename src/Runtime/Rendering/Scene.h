#pragma once
#include <memory>
#include <set>
#include <unordered_map>
#include <queue>

#include "Common/Components/CameraComponent.h"
#include "Common/Components/LightComponent.h"
#include "Common/Components/PrimitiveComponent.h"
#include "Common/Components/SkyAtmosphereComponent.h"
#include "Common/Components/SceneCaptureComponent.h"
#include "BatchedLine.h"
#include "Common/Delegate.h"
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

        FScene *Scene;
        UPrimitiveComponent *Primitive;
        FPrimitiveSceneProxy *SceneProxy;
        FRHIRenderQueryRef OcclusionQuery;
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

    class FSceneCaptureInfo
    {
        friend class FScene;
        friend class FDefferedShadingSceneRenderer;
    public:

        std::vector<FViewSceneInfo*> ViewSceneInfos;

        RHITexture* RenderTarget;

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

        // void AddSceneCapture(USceneCaptureComponent* InSceneCapture);
        // void RemoveSceneCapture(USceneCaptureComponent* InSceneCapture);

        void UpdateRenderInfos();

        TMulticastDelegate<FViewSceneInfo *> &GetAddViewDelegate() { return SceneAddViewDelegate; }
        TMulticastDelegate<FViewSceneInfo *> &GetRemoveViewDelegate() { return SceneRemoveViewDelegate; }
        TMulticastDelegate<FViewSceneInfo *> &GetResizeViewDelegate() { return SceneResizeViewortDelegate; }

        TMulticastDelegate<FLightSceneInfo *> &GetAddLightDelegate() { return SceneAddLightDelegate; }
        TMulticastDelegate<FLightSceneInfo *> &GetRemoveLightDelegate() { return SceneRemoveLightDelegate; }

        std::set<std::unique_ptr<FPrimitiveSceneInfo>> AddedPrimitiveSceneInfos;
        std::set<std::unique_ptr<FLightSceneInfo>> AddedLightSceneInfos;
        std::set<std::unique_ptr<FViewSceneInfo>> AddedViewSceneInfos;
        std::set<std::unique_ptr<FSceneCaptureInfo>> AddedSceneCaptureInfos;
        std::vector<std::unique_ptr<FSkyAtmosphereSceneProxy>> SkyAtmosphereStack;
        FSkyAtmosphereSceneProxy *SkyAtmosphere;
        class UWorld *World;

    protected:

        void AddPrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);
        void RemovePrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);

        void AddLightSceneInfo(FLightSceneInfo *InLightInfo);
        void RemoveLightSceneInfo(FLightSceneInfo *InLightInfo);

        void AddViewSceneInfo(FViewSceneInfo *InCameraInfo);
        void RemoveViewSceneInfo(FViewSceneInfo *InCameraInfo);

        void UpdateViewInfos();
        void UpdatePrimitiveInfos();
        void UpdateLightInfos();

        TMulticastDelegate<FViewSceneInfo *> SceneAddViewDelegate;
        TMulticastDelegate<FViewSceneInfo *> SceneRemoveViewDelegate;
        TMulticastDelegate<FViewSceneInfo *> SceneResizeViewortDelegate;

        TMulticastDelegate<FLightSceneInfo *> SceneAddLightDelegate;
        TMulticastDelegate<FLightSceneInfo *> SceneRemoveLightDelegate;
    };
}