#pragma once
#include <memory>
#include <set>
#include <unordered_map>

#include "Common/Components/CameraComponent.h"
#include "Common/Components/LightComponent.h"
#include "Common/Components/PrimitiveComponent.h"
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
        // TUniformBufferRef<FPrimitiveShaderParameters> PrimitiveShaderUniformBuffer;
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


    class FSceneTextures
    {
    public:
        RHITexture2DRef BaseColor;
        RHITexture2DRef WorldSpacePosition;
        RHITexture2DRef WorldSpaceNormal;
        RHITexture2DRef MetallicRoughness;
        RHITexture2DRef Emissive;
        RHITexture2DRef DepthStencil;
    };

    class FCameraSceneInfo
    {
    public:
        FCameraSceneInfo(FCameraSceneProxy *InSceneProxy, UCameraComponent *InCamera, FScene *InScene)
            : Camera(InCamera)
            , Scene(InScene)
            , SceneProxy(InSceneProxy)
        {

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
        UCameraComponent *Camera;
        FCameraSceneProxy *SceneProxy;
        RHIFramebufferRef FrameBuffer;
        FSceneTextures SceneTextures;
        bool bNeedsUniformBufferUpdate = false;
        bool bNeedsFramebufferUpdate = false;
    };

    class FScene
    {
    public:

        void AddCamera(UCameraComponent *InCamera);
        void RemoveCamera(UCameraComponent *InCamera);

        void AddLight(ULightComponent *InLight);
        void RemoveLight(ULightComponent *InLight);

        void AddPrimitive(UPrimitiveComponent *InPrimitive);
        void RemovePrimitive(UPrimitiveComponent *InPrimitive);
        // void UpdatePrimitiveTransform(UPrimitiveComponent *InPrimitive);

        // void UpdateAllPrimitiveSceneInfos();

        /** 下面两个在ue里是RenderThread */
        void AddPrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);
        void RemovePrimitiveSceneInfo(FPrimitiveSceneInfo *InPrimitiveInfo);

        void FScene::AddLightSceneInfo(FLightSceneInfo *InPrimitiveInfo);
        void FScene::RemoveLightSceneInfo(FLightSceneInfo *InPrimitiveInfo);

        void FScene::AddCameraSceneInfo(FCameraSceneInfo *InPrimitiveInfo);
        void FScene::RemoveCameraSceneInfo(FCameraSceneInfo *InPrimitiveInfo);

        void UpdateViewInfos();
        void UpdatePrimitiveInfos();

        std::set<FPrimitiveSceneInfo *> AddedPrimitiveSceneInfos;
        std::set<FLightSceneInfo *> AddedLightSceneInfos;
        std::vector<FCameraSceneInfo *> AddedCameraSceneInfos;
        FUniformBuffer LightUniformBuffer;
        class UWorld *World;
    };
}