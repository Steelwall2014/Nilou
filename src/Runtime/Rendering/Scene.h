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
            LightUniformBufferRHI = CreateUniformBuffer<FLightShaderParameters>();
        }

        FScene *Scene;
        ULightComponent *Light;
        FLightSceneProxy *SceneProxy;
        TUniformBufferRef<FLightShaderParameters> LightUniformBufferRHI;
    };

    class FViewSceneInfo
    {
    public:
        friend class FScene;
        friend class FDefferedShadingSceneRenderer;
        FViewSceneInfo(FCameraSceneProxy *InSceneProxy, UCameraComponent *InCamera, FScene *InScene)
            : Scene(InScene)
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

        FScene *Scene;
        FCameraSceneProxy *SceneProxy;
        std::shared_ptr<FViewElementPDI> PDI;
    private:
        bool bNeedsUniformBufferUpdate = false;
        bool bNeedsFramebufferUpdate = false;
    };

    enum class EShadingPath
    {
        Deferred,
        Num,
    };

    class FScene
    {
    public:

        FScene();

        uint32 GetFrameNumber() const
        {
            return SceneFrameNumber;
        }

        void IncrementFrameNumber()
        {
            ++SceneFrameNumber;
        }

        void AddSkyAtmosphere(FSkyAtmosphereSceneProxy *InSkyAtmosphereProxy);
        void RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy *InSkyAtmosphereProxy);

        void AddLight(ULightComponent *InLight);
        void RemoveLight(ULightComponent *InLight);

        void AddPrimitive(UPrimitiveComponent *InPrimitive);
        void RemovePrimitive(UPrimitiveComponent *InPrimitive);

        void UpdateRenderInfos();

        TMulticastDelegate<FLightSceneInfo *> &GetAddLightDelegate() { return SceneAddLightDelegate; }
        TMulticastDelegate<FLightSceneInfo *> &GetRemoveLightDelegate() { return SceneRemoveLightDelegate; }

        static EShadingPath GetShadingPath()
        {
            return EShadingPath::Deferred;
        }
        

        std::set<FPrimitiveSceneInfo*> AddedPrimitiveSceneInfos;
        std::set<FLightSceneInfo*> AddedLightSceneInfos;
        std::vector<std::unique_ptr<FSkyAtmosphereSceneProxy>> SkyAtmosphereStack;
        FSkyAtmosphereSceneProxy *SkyAtmosphere;
        class UWorld *World;

    protected:

        uint32 SceneFrameNumber = 0;

        void AddPrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo);
        void RemovePrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo);

        void AddLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo);
        void RemoveLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo);

        void UpdatePrimitiveInfos();
        void UpdateLightInfos();

        TMulticastDelegate<FLightSceneInfo *> SceneAddLightDelegate;
        TMulticastDelegate<FLightSceneInfo *> SceneRemoveLightDelegate;
    };
}