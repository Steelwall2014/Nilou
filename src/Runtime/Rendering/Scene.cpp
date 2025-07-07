#include <algorithm>


#include "DynamicRHI.h"
#include "Scene.h"
#include "BaseApplication.h"
#include "DeferredShadingSceneRenderer.h"
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
            [Scene, SkyAtmosphereSceneProxy](RenderGraph&)
            {
                Scene->SkyAtmosphereStack.emplace_back(SkyAtmosphereSceneProxy);
                Scene->SkyAtmosphere = SkyAtmosphereSceneProxy;
            });

    }

    void FScene::RemoveSkyAtmosphere(FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy)
    {
        FScene *Scene = this;

        ENQUEUE_RENDER_COMMAND(FScene_AddSkyAtmosphere)(
            [Scene, SkyAtmosphereSceneProxy](RenderGraph&)
            {
                safe_erase(Scene->SkyAtmosphereStack, SkyAtmosphereSceneProxy);
                if (!Scene->SkyAtmosphereStack.empty())
                    Scene->SkyAtmosphere = Scene->SkyAtmosphereStack.back().get();
                else
                    Scene->SkyAtmosphere = nullptr;
            });

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
            [Scene, LightSceneInfo] (RenderGraph&) 
            {
                Scene->AddLightSceneInfo_RenderThread(LightSceneInfo);
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
                [Scene, LightSceneInfo] (RenderGraph&) 
                {
                    Scene->RemoveLightSceneInfo_RenderThread(LightSceneInfo);
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
        FBoxSphereBounds Bounds = InPrimitive->GetBounds();

        FScene *Scene = this;
        ENQUEUE_RENDER_COMMAND(AddPrimitive)(
            [Scene, PrimitiveSceneProxy, PrimitiveSceneInfo, RenderMatrix, Bounds] (RenderGraph& Graph) 
            {
                PrimitiveSceneProxy->CreateUniformBuffer();
                PrimitiveSceneProxy->UpdateUniformBuffer(Graph);
                PrimitiveSceneProxy->CreateRenderThreadResources();
                Scene->AddPrimitiveSceneInfo_RenderThread(PrimitiveSceneInfo);
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
                [Scene, PrimitiveSceneProxy, PrimitiveSceneInfo] (RenderGraph&) 
                {
                    PrimitiveSceneInfo->SceneProxy->DestroyRenderThreadResources();
                    Scene->RemovePrimitiveSceneInfo_RenderThread(PrimitiveSceneInfo);
                });
        }
    }

    void FScene::AddReflectionProbe(UReflectionProbeComponent *InReflectionProbe)
    {
        FReflectionProbeSceneProxy *ReflectionProbeSceneProxy = InReflectionProbe->CreateSceneProxy();
        if (ReflectionProbeSceneProxy == nullptr)
            return;
            
        InReflectionProbe->SceneProxy = ReflectionProbeSceneProxy;
        FReflectionProbeSceneInfo *ReflectionProbeSceneInfo = new FReflectionProbeSceneInfo(ReflectionProbeSceneProxy, InReflectionProbe, this);
        ReflectionProbeSceneProxy->ReflectionProbeSceneInfo = ReflectionProbeSceneInfo;

        FBoxSphereBounds Bounds = InReflectionProbe->GetBounds();

        FScene *Scene = this;
        ENQUEUE_RENDER_COMMAND(AddReflectionProbe)(
            [Scene, ReflectionProbeSceneProxy, ReflectionProbeSceneInfo, Bounds] (RenderGraph&) 
            {
                Scene->AddReflectionProbeSceneInfo_RenderThread(ReflectionProbeSceneInfo);
            });
    }

    void FScene::RemoveReflectionProbe(UReflectionProbeComponent *InReflectionProbe)
    {
        FReflectionProbeSceneProxy* ReflectionProbeSceneProxy = InReflectionProbe->SceneProxy;

        if (ReflectionProbeSceneProxy)
        {
            FReflectionProbeSceneInfo* ReflectionProbeSceneInfo = ReflectionProbeSceneProxy->ReflectionProbeSceneInfo;
            InReflectionProbe->SceneProxy = nullptr;

            FScene *Scene = this;
            ENQUEUE_RENDER_COMMAND(RemoveReflectionProbe)(
                [Scene, ReflectionProbeSceneProxy, ReflectionProbeSceneInfo] (RenderGraph&) 
                {
                    Scene->RemoveReflectionProbeSceneInfo_RenderThread(ReflectionProbeSceneInfo);
                });
        }
    }

    void FScene::UpdateRenderInfos(RenderGraph& Graph)
    {
        UpdatePrimitiveInfos(Graph);
        UpdateLightInfos(Graph);
    }

    void FScene::UpdatePrimitiveTransform(UPrimitiveComponent *Primitive)
    {
        FPrimitiveSceneProxy* Proxy = Primitive->GetSceneProxy();
        if (Proxy)
        {
            glm::mat4 RenderMatrix = Primitive->GetRenderMatrix();
            FBoxSphereBounds Bounds = Primitive->GetBounds();
            ENQUEUE_RENDER_COMMAND(UpdatePrimitiveTransform)(
                [this, Proxy, RenderMatrix, Bounds](RenderGraph&)
                {
                    UpdatePrimitiveTransform_RenderThread(Proxy, RenderMatrix, Bounds);
                });
        }
    }

    void FScene::UpdatePrimitiveTransform_RenderThread(FPrimitiveSceneProxy *Proxy, const dmat4 &RenderMatrix, const FBoxSphereBounds &Bounds)
    {
        UpdatedTransforms[Proxy] = FUpdateTransformCommand(Bounds, Bounds, RenderMatrix);
    }

    void FScene::AddPrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        AddedPrimitiveSceneInfos.emplace(InPrimitiveInfo);
    }

    void FScene::RemovePrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo *InPrimitiveInfo)
    {
        AddedPrimitiveSceneInfos.erase(InPrimitiveInfo);
        delete InPrimitiveInfo;
    }

    void FScene::AddLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo)
    {
        AddedLightSceneInfos.emplace(InLightInfo);
        // InLightInfo->LightUniformBufferRHI->InitResource();
        GetAddLightDelegate().Broadcast(InLightInfo);
    }

    void FScene::RemoveLightSceneInfo_RenderThread(FLightSceneInfo *InLightInfo)
    {
        // InLightInfo->LightUniformBufferRHI->ReleaseResource();
        AddedLightSceneInfos.erase(InLightInfo);
        GetRemoveLightDelegate().Broadcast(InLightInfo);
        delete InLightInfo;
    }

    void FScene::AddReflectionProbeSceneInfo_RenderThread(FReflectionProbeSceneInfo *InReflectionProbeInfo)
    {
        ReflectionProbes.emplace(InReflectionProbeInfo);
    }

    void FScene::RemoveReflectionProbeSceneInfo_RenderThread(FReflectionProbeSceneInfo *InReflectionProbeInfo)
    {
        ReflectionProbes.erase(InReflectionProbeInfo);
        delete InReflectionProbeInfo;
    }

    void FScene::UpdatePrimitiveInfos(RenderGraph& Graph)
    {
        for (auto &&PrimitiveInfo : AddedPrimitiveSceneInfos)
        {
            if (PrimitiveInfo->bNeedsUniformBufferUpdate)
                PrimitiveInfo->SceneProxy->UpdateUniformBuffer(Graph);
        }
    }

    static void SetLightAttenParams(FLightAttenParameters &OutParameter, const FAttenCurve &AttenCurveParam)
    {
        EAttenCurveType CurveType = AttenCurveParam.type;
        OutParameter.AttenCurveType = (int)AttenCurveParam.type;
        OutParameter.AttenCurveScale = AttenCurveParam.scale;
        switch (CurveType) 
        {
            case EAttenCurveType::ACT_Linear:
                OutParameter.AttenCurveParams =
                    vec4(
                        AttenCurveParam.u.linear_params.begin_atten, 
                        AttenCurveParam.u.linear_params.end_atten, 
                        0, 0);
                break;
            case EAttenCurveType::ACT_Smooth:
                OutParameter.AttenCurveParams =
                    vec4(
                        AttenCurveParam.u.smooth_params.begin_atten, 
                        AttenCurveParam.u.smooth_params.end_atten, 
                        0, 0);
                break;
            case EAttenCurveType::ACT_Inverse:
                OutParameter.AttenCurveParams =
                    vec4(
                        AttenCurveParam.u.inverse_params.offset, 
                        AttenCurveParam.u.inverse_params.kl, 
                        AttenCurveParam.u.inverse_params.kc, 0);
                break;
            case EAttenCurveType::ACT_InverseSquare:
                OutParameter.AttenCurveParams =
                    vec4(
                        AttenCurveParam.u.inverse_squre_params.offset,
                        AttenCurveParam.u.inverse_squre_params.kq,
                        AttenCurveParam.u.inverse_squre_params.kl,
                        AttenCurveParam.u.inverse_squre_params.kc);
                break;
            default:
                Ncheck(0);
        }
    }

    static void SetLightUniformBuffer(RenderGraph& Graph, TRDGUniformBuffer<FLightShaderParameters>* LightUniformBuffer, FLightSceneProxy* Proxy)
    {
        FLightShaderParameters Parameters;
        Parameters.lightPosition = Proxy->Position;
        Parameters.lightDirection = Proxy->Direction;
        Parameters.lightIntensity = Proxy->LightIntensity;
        Parameters.lightCastShadow = Proxy->bCastShadow;
        Parameters.lightType = (int)Proxy->LightType;
        SetLightAttenParams(Parameters.lightDistAttenParams, Proxy->DistAttenCurve);
        SetLightAttenParams(Parameters.lightAngleAttenParams, Proxy->AngleAttenCurve);
        Graph.QueueBufferUpload(LightUniformBuffer, &Parameters, sizeof(Parameters));
    }

    void FScene::UpdateLightInfos(RenderGraph& Graph)
    {
        for (auto &&LightInfo : AddedLightSceneInfos)
        {
            FLightSceneProxy* Proxy = LightInfo->SceneProxy;
            SetLightUniformBuffer(Graph, LightInfo->LightUniformBuffer, Proxy);
        }
    }

    void FScene::Release_RenderThread()
    {
        std::set<FPrimitiveSceneInfo*> PrimitivesToDelete = AddedPrimitiveSceneInfos;
        std::set<FLightSceneInfo*> LightsToDelete = AddedLightSceneInfos;
        std::set<FReflectionProbeSceneInfo*> ReflectionProbesToDelete = ReflectionProbes;
        for (FPrimitiveSceneInfo* PrimitiveInfo : PrimitivesToDelete)
            RemovePrimitiveSceneInfo_RenderThread(PrimitiveInfo);
        for (FLightSceneInfo* LightInfo : LightsToDelete)
            RemoveLightSceneInfo_RenderThread(LightInfo);
        for (FReflectionProbeSceneInfo* ProbeInfo : ReflectionProbesToDelete)
            RemoveReflectionProbeSceneInfo_RenderThread(ProbeInfo);
        SkyAtmosphereStack.clear();
    }
}