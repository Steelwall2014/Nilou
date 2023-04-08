#include "SceneView.h"

namespace nilou {

    FSceneView::FSceneView()
        : FSceneView(
            glm::radians(90.0), 
            0.1, 
            10000, 
            dvec3(0), 
            dvec3(1, 0, 0),
            dvec3(0, 0, 1),
            ivec2(1920, 1080),
            nullptr)
    {

    }

    FSceneView::FSceneView(
        double InVerticalFieldOfView, 
        double InNearClipDistance, 
        double InFarClipDistance,
        dvec3 InPosition,
        dvec3 InForward,
        dvec3 InUp,
        ivec2 InScreenResolution,
        TUniformBufferRef<FViewShaderParameters> InViewUniformBuffer)
        : VerticalFieldOfView(InVerticalFieldOfView)
        , NearClipDistance(InNearClipDistance)
        , FarClipDistance(InFarClipDistance)
        , Position(InPosition)
        , Forward(InForward)
        , Up(InUp)
        , AspectRatio(double(InScreenResolution.x) / double(InScreenResolution.y))
        , ScreenResolution(InScreenResolution)
        , ViewUniformBuffer(InViewUniformBuffer)
    { 
        ViewMatrix = glm::lookAt(
            Position, 
            Position+Forward, 
            Up);
        ProjectionMatrix = glm::perspective(
            VerticalFieldOfView, 
            AspectRatio, 
            NearClipDistance, 
            FarClipDistance);
        ViewFrustum = FViewFrustum(
            Position, 
            Forward, 
            Up, 
            AspectRatio, 
            VerticalFieldOfView, 
            NearClipDistance, 
            FarClipDistance);
            
        if (ViewUniformBuffer)
        {
            const dmat4& WorldToView = ViewMatrix;
            const mat4& ViewToClip = ProjectionMatrix;
            mat4 RelativeWorldToView = WorldToView;
            RelativeWorldToView[3][0] = 0;
            RelativeWorldToView[3][1] = 0;
            RelativeWorldToView[3][2] = 0;
            ViewUniformBuffer->Data.RelWorldToView = RelativeWorldToView;
            ViewUniformBuffer->Data.ViewToClip = ViewToClip;
            ViewUniformBuffer->Data.RelWorldToClip = ViewToClip * RelativeWorldToView;
            ViewUniformBuffer->Data.ClipToView = glm::inverse(ViewToClip);
            ViewUniformBuffer->Data.RelClipToWorld = glm::inverse(ViewToClip * RelativeWorldToView);
            ViewUniformBuffer->Data.AbsWorldToClip = ViewToClip * mat4(WorldToView);

            ViewUniformBuffer->Data.CameraPosition = Position;
            ViewUniformBuffer->Data.CameraDirection = Forward;
            ViewUniformBuffer->Data.CameraResolution = ScreenResolution;
            ViewUniformBuffer->Data.CameraNearClipDist = NearClipDistance;
            ViewUniformBuffer->Data.CameraFarClipDist = FarClipDistance;
            ViewUniformBuffer->Data.CameraVerticalFieldOfView = VerticalFieldOfView;

            for (int i = 0; i < 6; i++)
                ViewUniformBuffer->Data.FrustumPlanes[i] = dvec4(ViewFrustum.Planes[i].Normal, ViewFrustum.Planes[i].Distance);

        }
    }

}