#include "SceneView.h"

namespace nilou {

    FSceneView::FSceneView()
        : FSceneView(
            ECameraProjectionMode::Perspective,
            glm::radians(90.0), 
            0,
            0.1, 
            10000, 
            dvec3(0), 
            dvec3(1, 0, 0),
            dvec3(0, 0, 1),
            ivec2(1920, 1080))
    {

    }

    FSceneView::FSceneView(
        ECameraProjectionMode InProjectionMode,
        double InVerticalFieldOfView, 
        double InOrthoWidth,
        double InNearClipDistance, 
        double InFarClipDistance,
        dvec3 InPosition,
        dvec3 InForward,
        dvec3 InUp,
        ivec2 InScreenResolution)
        : VerticalFieldOfView(InVerticalFieldOfView)
        , OrthoWidth(InOrthoWidth)
        , ProjectionMode(InProjectionMode)
        , NearClipDistance(InNearClipDistance)
        , FarClipDistance(InFarClipDistance)
        , Position(InPosition)
        , Forward(InForward)
        , Up(InUp)
        , AspectRatio(double(InScreenResolution.x) / double(InScreenResolution.y))
        , ScreenResolution(InScreenResolution)
    { 
        ViewMatrix = glm::lookAt(
            Position, 
            Position+Forward, 
            Up);
        if (ProjectionMode == ECameraProjectionMode::Perspective)
        {
            ProjectionMatrix = glm::perspective(
                VerticalFieldOfView, 
                AspectRatio, 
                NearClipDistance, 
                FarClipDistance);
        }
        else if (ProjectionMode == ECameraProjectionMode::Orthographic)
        {
            double OrthoHeight = OrthoWidth / AspectRatio;
            ProjectionMatrix = glm::ortho(
                -OrthoWidth*0.5, 
                OrthoWidth*0.5, 
                -OrthoHeight*0.5, 
                OrthoHeight*0.5,
                NearClipDistance, 
                FarClipDistance);
        }
        
        // ViewFrustum = FViewFrustum(
        //     Position, 
        //     Forward, 
        //     Up, 
        //     AspectRatio, 
        //     VerticalFieldOfView, 
        //     NearClipDistance, 
        //     FarClipDistance);
        
        ViewFrustum = FViewFrustum(ViewMatrix, ProjectionMatrix);
            
        if (ViewUniformBuffer)
        {
            const dmat4& WorldToView = ViewMatrix;
            const mat4& ViewToClip = ProjectionMatrix;
            mat4 RelativeWorldToView = WorldToView;
            RelativeWorldToView[3][0] = 0;
            RelativeWorldToView[3][1] = 0;
            RelativeWorldToView[3][2] = 0;
            FViewShaderParameters Parameters;
            Parameters.RelWorldToView = RelativeWorldToView;
            Parameters.ViewToClip = ViewToClip;
            Parameters.RelWorldToClip = ViewToClip * RelativeWorldToView;
            Parameters.ClipToView = glm::inverse(ViewToClip);
            Parameters.RelClipToWorld = glm::inverse(ViewToClip * RelativeWorldToView);
            Parameters.AbsWorldToClip = ViewToClip * mat4(WorldToView);

            Parameters.CameraPosition = Position;
            Parameters.CameraDirection = Forward;
            Parameters.CameraResolution = ScreenResolution;
            Parameters.CameraNearClipDist = NearClipDistance;
            Parameters.CameraFarClipDist = FarClipDistance;
            Parameters.CameraVerticalFieldOfView = VerticalFieldOfView;

            for (int i = 0; i < 6; i++)
            {
                Parameters.FrustumPlanes[i] = dvec4(ViewFrustum.Planes[i].Normal, ViewFrustum.Planes[i].Distance);
            }

            ViewUniformBuffer->UpdateUniformBufferImmediate(Parameters);
        }
    }

    FSceneViewFamily::FSceneViewFamily(
        FViewport InViewport, 
        FScene* InScene)
        : Viewport(InViewport)
        , Scene(InScene)
        , FrameNumber(0)
        , GammaCorrection(1.0)
        , bEnableToneMapping(false)
        , bIsSceneCapture(false)
    {

    }

    FSceneViewFamily::FSceneViewFamily(const FSceneViewFamily& Other)
        : Viewport(Other.Viewport)
        , Scene(Other.Scene)
        , FrameNumber(Other.FrameNumber)
        , HiddenComponents(Other.HiddenComponents)
        , ShowOnlyComponents(Other.ShowOnlyComponents)
        , GammaCorrection(Other.GammaCorrection)
        , Views(Other.Views)
        , bEnableToneMapping(Other.bEnableToneMapping)
        , bIsSceneCapture(Other.bIsSceneCapture)
        , CaptureSource(Other.CaptureSource)
    {

    }

}