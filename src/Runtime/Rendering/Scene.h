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
#include "Common/Components/ReflectionProbeComponent.h"
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

        ~FPrimitiveSceneInfo()
        {
            delete SceneProxy;
        }

        void SetNeedsUniformBufferUpdate(bool bInNeedsUniformBufferUpdate)
        {
            bNeedsUniformBufferUpdate = bInNeedsUniformBufferUpdate;
        }

        FScene *Scene;
        UPrimitiveComponent *Primitive;
        FPrimitiveSceneProxy *SceneProxy;
        std::map<FReflectionProbeSceneInfo*, float> ReflectionProbeFactors;
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
            LightUniformBuffer = RenderGraph::CreateExternalUniformBuffer<FLightShaderParameters>("", nullptr);
        }

        ~FLightSceneInfo()
        {
            delete SceneProxy;
        }

        FScene *Scene;
        ULightComponent *Light;
        FLightSceneProxy *SceneProxy;
        TRDGUniformBufferRef<FLightShaderParameters> LightUniformBuffer;
    };

    class FReflectionProbeSceneInfo
    {
    public:
        FReflectionProbeSceneInfo(FReflectionProbeSceneProxy *InSceneProxy, UReflectionProbeComponent *InReflectionProbe, FScene *InScene)
            : ReflectionProbe(InReflectionProbe)
            , Scene(InScene)
            , SceneProxy(InSceneProxy)
        {

        }

        ~FReflectionProbeSceneInfo()
        {
            delete SceneProxy;
        }

        FScene *Scene;
        UReflectionProbeComponent *ReflectionProbe;
        FReflectionProbeSceneProxy *SceneProxy;

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

        void AddReflectionProbe(UReflectionProbeComponent *InReflectionProbe);
        void RemoveReflectionProbe(UReflectionProbeComponent *InReflectionProbe);

        void UpdateRenderInfos(RenderGraph& Graph);

        void UpdatePrimitiveTransform(UPrimitiveComponent *Primitive);
        void UpdatePrimitiveTransform_RenderThread(FPrimitiveSceneProxy *Proxy, const dmat4 &RenderMatrix, const FBoxSphereBounds &Bounds);

        TMulticastDelegate<FLightSceneInfo *> &GetAddLightDelegate() { return SceneAddLightDelegate; }
        TMulticastDelegate<FLightSceneInfo *> &GetRemoveLightDelegate() { return SceneRemoveLightDelegate; }

        static EShadingPath GetShadingPath()
        {
            return EShadingPath::Deferred;
        }

        void Release_RenderThread();
        

        std::set<FPrimitiveSceneInfo*> AddedPrimitiveSceneInfos;
        std::set<FLightSceneInfo*> AddedLightSceneInfos;
        std::set<FReflectionProbeSceneInfo*> ReflectionProbes;
        std::vector<std::unique_ptr<FSkyAtmosphereSceneProxy>> SkyAtmosphereStack;
        FSkyAtmosphereSceneProxy *SkyAtmosphere;
        class UWorld *World;

    protected:

        uint32 SceneFrameNumber = 0;

        void AddPrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo);
        void RemovePrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo);

        void AddLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo);
        void RemoveLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo);

        void AddReflectionProbeSceneInfo_RenderThread(FReflectionProbeSceneInfo *InReflectionProbeInfo);
        void RemoveReflectionProbeSceneInfo_RenderThread(FReflectionProbeSceneInfo *InReflectionProbeInfo);

        void UpdatePrimitiveInfos(RenderGraph& Graph);
        void UpdateLightInfos(RenderGraph& Graph);

        TMulticastDelegate<FLightSceneInfo *> SceneAddLightDelegate;
        TMulticastDelegate<FLightSceneInfo *> SceneRemoveLightDelegate;

        struct FUpdateTransformCommand
        {
            FBoxSphereBounds WorldBounds;
            FBoxSphereBounds LocalBounds; 
            glm::mat4 LocalToWorld; 
        };
        std::unordered_map<FPrimitiveSceneProxy*, FUpdateTransformCommand> UpdatedTransforms;
    };
}