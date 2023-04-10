#pragma once

#include <vector>
#include "Frustum.h"
#include "UniformBuffer.h"
#include "MeshBatch.h"
#include "TextureRenderTarget.h"
#include "Common/Viewport.h"
// #include "Scene.h"

namespace nilou {

    // enum class EViewType
    // {
    //     CT_Perspective,
    //     CT_Ortho,
    // };

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
        double VerticalFieldOfView;
        double NearClipDistance;
        double FarClipDistance;
        glm::ivec2 ScreenResolution;

        /** 
         * The width of orthodox views. If it equals zero, it means the view is not orthodox. 
         * Orthodox height will be calculated with AspectRatio.
        */
        double OrthoWidth = 0.0;

        FSceneView();

        FSceneView(
            double InVerticalFieldOfView, 
            double InNearClipDistance, 
            double InFarClipDistance,
            dvec3 InPosition,
            dvec3 InForward,
            dvec3 InUp,
            ivec2 InScreenResolution,
            TUniformBufferRef<FViewShaderParameters> InViewUniformBuffer);

        // EViewType ViewType;
        
        TUniformBufferRef<FViewShaderParameters> ViewUniformBuffer;
        std::vector<FMeshBatch> DynamicMeshBatches;
    };

    /**
     * 一个对UE5 FSceneViewFamily的模仿，虽然我并不知道UE5的这个玩意有啥用
     */
    class FSceneViewFamily
    {
    public:

        FSceneViewFamily(
            FViewport InViewport, 
            class FScene* InScene)
            : Viewport(InViewport)
            , Scene(InScene)
            , FrameNumber(0)
        {

        }

        FViewport Viewport;

        FScene* Scene;

        std::vector<FSceneView*> Views;

        uint32 FrameNumber;

        std::set<class UPrimitiveComponent*> HiddenComponents;
    };
}