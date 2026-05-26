#pragma once

#include <vector>
#include <set>
#include "Frustum.h"
#include "UniformBuffer.h"
#include "MeshBatch.h"
#include "Viewport.h"
#include "EngineTypes.h"
#include "RenderGraphResources.h"
#include "RenderGraphParameterBlock.h"
#include "VertexShaderCommon.generated.h"

namespace nilou {

    // BEGIN_UNIFORM_BUFFER_STRUCT(shader::FViewShaderParameters)
    //     SHADER_PARAMETER_ARRAY(FVector4f, FrustumPlanes, [6])
    //     SHADER_PARAMETER(FMatrix44f, RelWorldToView)
    //     SHADER_PARAMETER(FMatrix44f, ViewToClip)
    //     SHADER_PARAMETER(FMatrix44f, RelWorldToClip)      // CPU: ViewToClip * RelWorldToView; shader row-major upload
    //     SHADER_PARAMETER(FMatrix44f, ClipToView)  
    //     SHADER_PARAMETER(FMatrix44f, RelClipToWorld)      // Inverse of RelWorldToClip
    //     SHADER_PARAMETER(FMatrix44f, ScreenToRelativeWorld) // ScreenToClip * RelClipToWorld (UE ScreenToTranslatedWorld)
    //     SHADER_PARAMETER(FMatrix44f, AbsWorldToClip)     
    //     SHADER_PARAMETER(FVector3f, CameraPosition)
    //     SHADER_PARAMETER(FVector3f, CameraDirection)
    //     SHADER_PARAMETER(uint32, bIsOrthoProjection)
    //     SHADER_PARAMETER(FVector2f, ViewRectMin)
    //     SHADER_PARAMETER(FVector4f, ViewSizeAndInvSize)
    //     SHADER_PARAMETER(FIntVector2, CameraResolution)
    //     SHADER_PARAMETER(float, CameraNearClipDist)
    //     SHADER_PARAMETER(float, CameraFarClipDist)
    //     SHADER_PARAMETER(float, CameraVerticalFieldOfView)
    // END_UNIFORM_BUFFER_STRUCT()
    
    class FSceneView 
    {
    public:
        /** The six planes of the view frustum */
        FViewFrustum ViewFrustum;

        FMatrix ProjectionMatrix;
        FMatrix ViewMatrix;
        FVector Position;
        FVector Forward;
        FVector Up;
        double AspectRatio;
        double NearClipDistance;
        double FarClipDistance;
        FIntVector2 ScreenResolution;

        ECameraProjectionMode ProjectionMode;
        double OrthoWidth;
        double VerticalFieldOfView;

        TParameterBlock<shader::FViewShaderParameters>* ViewUniformBuffer;

        FSceneView();

        FSceneView(
            ECameraProjectionMode InProjectionMode,
            double InVerticalFieldOfView,   // omitted if projection mode is Orthographic
            double InOrthoWidth,    // omitted if projection mode is Perspective
            double InNearClipDistance, 
            double InFarClipDistance,
            FVector InPosition,
            FVector InForward,
            FVector InUp,
            FIntVector2 InScreenResolution/*,
            TUniformBufferRef<shader::FViewShaderParameters> InViewUniformBuffer*/);

        // EViewType ViewType;
        
        /*TUniformBufferRef<shader::FViewShaderParameters> ViewUniformBuffer;
        std::vector<FMeshBatch> DynamicMeshBatches;*/
    };

    /**
     * 一个对UE5 FSceneViewFamily的模仿，虽然我并不知道UE5的这个玩意有啥用
     * A set of views into a scene which only have different view transforms and owner actors.
     */
    class FSceneViewFamily
    {
    public:

        FSceneViewFamily(FViewport InViewport, class FScene* InScene);

        FSceneViewFamily(const FSceneViewFamily& Other);

        FViewport Viewport;

        FScene* Scene;

        std::vector<FSceneView> Views;

        uint32 FrameNumber;

        std::set<class UPrimitiveComponent*> HiddenComponents;

        std::set<class UPrimitiveComponent*> ShowOnlyComponents;

	    /** Gamma correction used when rendering this family. Default is 1.0 */
        float GammaCorrection;

        bool bEnableToneMapping;

        bool bIsSceneCapture;

        ESceneCaptureSource CaptureSource;  // ignored if bIsSceneCapture is false

    };
}