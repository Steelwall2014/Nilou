#pragma once

#include <vector>
#include "Frustum.h"
#include "UniformBuffer.h"
#include "MeshBatch.h"
#include "TextureRenderTarget.h"
#include "Common/Viewport.h"
#include "Common/EngineTypes.h"

namespace nilou {

    BEGIN_UNIFORM_BUFFER_STRUCT(FViewShaderParameters)
        SHADER_PARAMETER_ARRAY(dvec4, 6, FrustumPlanes)
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
    
    class FSceneView 
    {
    public:
        /** The six planes of the view frustum */
        FViewFrustum ViewFrustum;

        glm::dmat4 ProjectionMatrix;
        glm::dmat4 ViewMatrix;
        dvec3 Position;
        dvec3 Forward;
        dvec3 Up;
        double AspectRatio;
        double NearClipDistance;
        double FarClipDistance;
        ivec2 ScreenResolution;

        ECameraProjectionMode ProjectionMode;
        double OrthoWidth;
        double VerticalFieldOfView;

        RDGBuffer* ViewUniformBuffer;

        FSceneView();

        FSceneView(
            ECameraProjectionMode InProjectionMode,
            double InVerticalFieldOfView,   // omitted if projection mode is Orthographic
            double InOrthoWidth,    // omitted if projection mode is Perspective
            double InNearClipDistance, 
            double InFarClipDistance,
            dvec3 InPosition,
            dvec3 InForward,
            dvec3 InUp,
            ivec2 InScreenResolution/*,
            TUniformBufferRef<FViewShaderParameters> InViewUniformBuffer*/);

        // EViewType ViewType;
        
        /*TUniformBufferRef<FViewShaderParameters> ViewUniformBuffer;
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