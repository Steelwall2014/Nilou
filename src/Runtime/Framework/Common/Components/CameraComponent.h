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
            , VerticalFieldOfView(glm::radians(50.f))
            , NearClipDistance(0.1)
            , FarClipDistance(10000)
            , AspectRatio(1.f)
            , ScreenResolution(glm::ivec2(1024, 1024))
        { 
        }

        virtual class FCameraSceneProxy *CreateSceneProxy(); 

        virtual void OnRegister() override;
        virtual void OnUnregister() override;

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        virtual void SendRenderDynamicData() override;

        glm::mat4 CalcWorldToViewMatrix();

        glm::mat4 CalcViewToClipMatrix();

        bool IsMainCamera() { return bIsMainCamera; }

        float GetAspectRatio() const { return AspectRatio; }

        float GetNearClipDistance() const { return NearClipDistance; }

        float GetFarClipDistance() const { return FarClipDistance; }

        float GetFieldOfView() const { return VerticalFieldOfView; }
        /**
         * Set the vertical field of view. This will call MarkRenderDynamicDataDirty().
         * 
         * @param InVerticalFieldOfView The angle of vertical FOV, in radians
         */
        void SetFieldOfView(float InVerticalFieldOfView);

        ivec2 GetCameraResolution() const { return ScreenResolution; }

        void SetCameraResolution(const ivec2 &CameraResolution);

        FViewFrustum CalcViewFrustum();

    protected:

        class FCameraSceneProxy *SceneProxy;

        /** Field of view. in radians */
        float VerticalFieldOfView;
        float NearClipDistance;
        float FarClipDistance;
        /** AspectRatio = ScreenWidth/ScreenHeight */
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

        void SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp);

        void SetViewProjectionMatrix(const glm::dmat4 &InWorldToView, const glm::mat4 &InViewToClip);

        void SetCameraResolution(const ivec2 &InCameraResolution);

        void SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist);
    
        void UpdateUniformBuffer() 
        { 
            ViewUniformBufferRHI->UpdateUniformBuffer(); 
            SceneView.ViewFrustum = FViewFrustum(
                SceneView.Position, SceneView.Forward, SceneView.Up, 
                SceneView.AspectRatio, SceneView.VerticalFieldOfView, 
                SceneView.NearClipDistance, SceneView.FarClipDistance); 
        }

        TUniformBuffer<FViewShaderParameters> *GetViewUniformBuffer() { return ViewUniformBufferRHI.get(); }

        const FSceneView &GetSceneView();

    protected:
        FSceneView SceneView;
        FCameraSceneInfo *CameraSceneInfo;
        TUniformBufferRef<FViewShaderParameters> ViewUniformBufferRHI;
    };
}