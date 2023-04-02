#pragma once
#include "SceneComponent.h"
#include "SceneView.h"
#include "Templates/ObjectMacros.h"
#include "RenderingThread.h"

namespace nilou {

    // struct FCameraParameters
    // {
    //     // EViewType CameraType;

    //     FCameraParameters();
    // };

    UCLASS()
    class UCameraComponent : public USceneComponent
    {
        friend class FScene;
        friend class FCameraSceneProxy;
        GENERATE_CLASS_INFO()
    public:

        UCameraComponent(AActor *InOwner=nullptr);

        virtual class FCameraSceneProxy *CreateSceneProxy(); 

        virtual void OnRegister() override;
        virtual void OnUnregister() override;

        virtual void CreateRenderState() override;

        virtual void DestroyRenderState() override;

        virtual void SendRenderTransform() override;

        glm::dmat4 CalcWorldToViewMatrix();

        glm::mat4 CalcViewToClipMatrix();

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

        double GetMaxCascadeShadowMapDistance() const { return MaxCascadeShadowMapDistance; }

        void SetMaxCascadeShadowMapDistance(double MaxCascadeShadowMapDistance);

        FViewFrustum CalcViewFrustum();

        FCameraSceneProxy* GetSceneProxy() const { return SceneProxy; }

    protected:

        class FCameraSceneProxy *SceneProxy;

        /** Field of view. in radians */
        float VerticalFieldOfView;
        float NearClipDistance;
        float FarClipDistance;
        /** AspectRatio = ScreenWidth/ScreenHeight */
        float AspectRatio;
        glm::ivec2 ScreenResolution;

        double MaxCascadeShadowMapDistance = 800;
    };

    BEGIN_UNIFORM_BUFFER_STRUCT(FViewShaderParameters)
        SHADER_PARAMETER_STRUCT_ARRAY(dvec4, 6, FrustumPlanes)
        SHADER_PARAMETER(mat4, RelWorldToView)
        SHADER_PARAMETER(mat4, ViewToClip)
        SHADER_PARAMETER(mat4, RelWorldToClip)      // RelWorldToClip = ViewToClip * RelWorldToView
        SHADER_PARAMETER(mat4, ClipToView)  
        SHADER_PARAMETER(mat4, RelClipToWorld)      // Inverse of RelWorldToClip
        SHADER_PARAMETER(mat4, AbsWorldToClip)     
        SHADER_PARAMETER(dvec3, CameraPosition)
        SHADER_PARAMETER(vec3, CameraDirection)
        SHADER_PARAMETER(ivec2, CameraResolution)
        SHADER_PARAMETER(float, CameraNearClipDist)
        SHADER_PARAMETER(float, CameraFarClipDist)
        SHADER_PARAMETER(float, CameraVerticalFieldOfView)
    END_UNIFORM_BUFFER_STRUCT()

    class FViewSceneInfo;
    class FCameraSceneProxy
    {
        friend class FScene;
    public:

        FCameraSceneProxy(UCameraComponent *InComponent);

        FViewSceneInfo *GetViewSceneInfo() { return ViewSceneInfo; }

        void SetPositionAndDirection(const glm::dvec3 &InPosition, const glm::vec3 &InDirection, const glm::vec3 &InUp);

        void SetViewProjectionMatrix(const glm::dmat4 &InWorldToView, const glm::mat4 &InViewToClip);

        void SetCameraResolution(const ivec2 &InCameraResolution);

        void SetCameraClipDistances(float InCameraNearClipDist, float InCameraFarClipDist);

        void SetFieldOfView(float InVerticalFieldOfView);

        void UpdateFrustum();
    
        void UpdateUniformBuffer() 
        { 
            ENQUEUE_RENDER_COMMAND(FCameraSceneProxy_UpdateUniformBuffer)(
                [this](FDynamicRHI *DynamicRHI) 
                {
                    ViewUniformBufferRHI->UpdateUniformBuffer();
                }); 
        }

        TUniformBuffer<FViewShaderParameters> *GetViewUniformBuffer() { return ViewUniformBufferRHI.get(); }

        FSceneView GetSceneView();

        FViewFrustum ViewFrustum;
        glm::dmat4 ProjectionMatrix;
        glm::dmat4 ViewMatrix;
        dvec3 Position;
        dvec3 Forward;
        dvec3 Up;
        double AspectRatio;
        double VerticalFieldOfView;
        double NearClipDistance;
        double FarClipDistance;
        glm::ivec2 ScreenResolution;
        EViewType ViewType;
        double MaxCascadeShadowMapDistance;
    
    protected:
        FViewSceneInfo *ViewSceneInfo;
        TUniformBufferRef<FViewShaderParameters> ViewUniformBufferRHI;
    };
}