#pragma once
#include "SceneComponent.h"
#include "SceneView.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    enum class ECameraType
    {
        CT_Perspective,
        CT_Ortho,
    };

    // struct FCameraParameters
    // {
    //     // ECameraType CameraType;

    //     FCameraParameters();
    // };

    UCLASS()
    class UCameraComponent : public USceneComponent
    {
        friend class FScene;
        friend class FCameraSceneProxy;
        GENERATE_CLASS_INFO()
    public:

        UCameraComponent(AActor *InOwner, bool bIsMainCamera=false) 
            : USceneComponent(InOwner)
            , SceneProxy(nullptr)
            , bIsMainCamera(bIsMainCamera)
            , FOVY(50)
            , NearClipDistance(0.1)
            , FarClipDistance(10000)
            , AspectRatio(1.f)
            , ScreenResolution(glm::ivec2(1024, 1024))
        { 
        }

        virtual class FCameraSceneProxy *CreateSceneProxy(); 

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        virtual void SendRenderDynamicData() override;

        glm::mat4 CalcWorldToViewMatrix();

        glm::mat4 CalcViewToClipMatrix();

        bool IsMainCamera() { return bIsMainCamera; }

        void SetFieldOfView(float fovy);

        void SetCameraResolution(const ivec2 &CameraResolution);

    protected:

        class FCameraSceneProxy *SceneProxy;

        /** Field of view. !!IN DEGREES!! */
        float FOVY;
        float NearClipDistance;
        float FarClipDistance;
        float AspectRatio;
        glm::ivec2 ScreenResolution;

        bool bIsMainCamera;

        bool bCameraResolutionDirty = false;
    };

    BEGIN_UNIFORM_BUFFER_STRUCT(FViewShaderParameters)
        SHADER_PARAMETER(mat4, WorldToView)
        SHADER_PARAMETER(mat4, ViewToClip)
        SHADER_PARAMETER(mat4, WorldToClip) // WorldToClip = ViewToClip * WorldToView
        SHADER_PARAMETER(mat4, ClipToWorld) // Inverse of WorldToClip
        SHADER_PARAMETER(dvec3, CameraPosition)
        SHADER_PARAMETER(vec3, CameraDirection)
        SHADER_PARAMETER(ivec2, CameraResolution)
        SHADER_PARAMETER(float, CameraNearClipDist)
        SHADER_PARAMETER(float, CameraFarClipDist)
    END_UNIFORM_BUFFER_STRUCT()

    class FCameraSceneInfo;
    class FCameraSceneProxy
    {
        friend class FScene;
    public:

        FCameraSceneProxy(UCameraComponent *InComponent);

        FCameraSceneInfo *GetCameraSceneInfo() { return CameraSceneInfo; }

        void SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection);

        void SetViewProjectionMatrix(const glm::dmat4 &InWorldToView, const glm::mat4 &InViewToClip);

        void SetCameraResolution(const ivec2 &InCameraResolution);

        void SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist);
    
        void UpdateUniformBuffer() { ViewUniformBufferRHI->UpdateUniformBuffer(); }

        TUniformBuffer<FViewShaderParameters> *GetViewUniformBuffer() { return ViewUniformBufferRHI.get(); }

        FSceneView SceneView;
        
    protected:
        FCameraSceneInfo *CameraSceneInfo;
        TUniformBufferRef<FViewShaderParameters> ViewUniformBufferRHI;
    };
}