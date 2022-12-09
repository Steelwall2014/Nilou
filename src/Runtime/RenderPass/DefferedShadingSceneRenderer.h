#pragma once

#include "SkyAtmospherePassRendering.h"

#include "Common/MeshBatch.h"
#include "Common/Scene.h"
#include "MeshPassProcessor.h"
#include "RHIResources.h"
#include "SceneView.h"
#include <unordered_map>
#include <vector>


namespace nilou {

    class FViewMeshBatches
    {
    public:
        std::vector<FMeshBatch> MeshBatches;
    };

    class FParallelMeshDrawCommands
    {
    public:
        void AddMeshDrawCommand(const FMeshDrawCommand &MeshDrawCommand);
        void Clear();
        void DispatchDraw(FDynamicRHI *RHICmdList);
    private:
        std::vector<FMeshDrawCommand> MeshCommands;
    };

    class FSceneTextures
    {
    public:
        RHIFramebufferRef FrameBuffer;
        RHITexture2DRef BaseColor;
        RHITexture2DRef WorldSpacePosition;
        RHITexture2DRef WorldSpaceNormal;
        RHITexture2DRef MetallicRoughness;
        RHITexture2DRef Emissive;
        RHITexture2DRef DepthStencil;
        RHITexture2DRef SceneColor;
    };

    // class FViewCommands
    // {
    // public:
    //     std::unordered_map<FMeshPassProcessor *, FParallelMeshDrawCommands> PerPassMeshCommands;
    // };

    class FSceneRenderer
    {
    public:
        static FSceneRenderer *CreateSceneRenderer(FScene *Scene);
    };

    class FDefferedShadingSceneRenderer : public FSceneRenderer
    {
    public:

        // FDefferedShadingSceneRenderer();
        FDefferedShadingSceneRenderer(FScene *Scene);

        virtual void Render();

        virtual void AddCamera(FCameraSceneInfo *CameraInfo);
        virtual void RemoveCamera(FCameraSceneInfo *CameraInfo);

    private:

        class FScreenQuadPositionVertexBuffer : public FVertexBuffer
        {
        public:
            FScreenQuadPositionVertexBuffer()
            {
                Vertices[0] = glm::vec4(-1, 1, 0, 1);
                Vertices[1] = glm::vec4(-1, -1, 0, 1);
                Vertices[2] = glm::vec4(1, 1, 0, 1);
                Vertices[3] = glm::vec4(1, -1, 0, 1);
            }
            virtual void InitRHI() override
            {
                VertexBufferRHI = GDynamicRHI->RHICreateBuffer(sizeof(glm::vec4), sizeof(Vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, Vertices);
            }
        private:
            glm::vec4 Vertices[4];
        };

        class FScreenQuadUVVertexBuffer : public FVertexBuffer
        {
        public:
            FScreenQuadUVVertexBuffer()
            {
                Vertices[0] = glm::vec2(0, 1);
                Vertices[1] = glm::vec2(0, 0);
                Vertices[2] = glm::vec2(1, 1);
                Vertices[3] = glm::vec2(1, 0);
            }
            virtual void InitRHI() override
            {
                VertexBufferRHI = GDynamicRHI->RHICreateBuffer(sizeof(glm::vec2), sizeof(Vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, Vertices);
            }
        private:
            glm::vec2 Vertices[4];
        };

        // void SetupMeshPass(FSceneView &View, const std::vector<FMeshBatch> &ViewMeshBatches, std::vector<FMeshDrawCommand> &OutMeshDrawCommands);
        void InitViews(FScene *Scene);

        void ComputeVisibility(FScene *Scene);

        void RenderBasePass(FDynamicRHI *RHICmdList);
        
        void RenderCSMShadowPass(FDynamicRHI *RHICmdList);

        void RenderLightingPass(FDynamicRHI *RHICmdList);

        FScene *Scene = nullptr;
        
        std::map<FCameraSceneInfo *, FSceneTextures> PerViewSceneTextures;
        std::map<FCameraSceneInfo *, std::vector<FMeshBatch>> PerViewMeshBatches;

        // std::vector<FViewCommands> PerViewDrawCommands;

        std::vector<RHIFramebufferRef> PerViewCSMRenderTarget;
        std::vector<std::vector<RHITexture2DRef>> PerViewCSMDepthBuffer;

                    
        FScreenQuadPositionVertexBuffer PositionVertexBuffer;
        FScreenQuadUVVertexBuffer UVVertexBuffer;
        FRHIVertexInput PositionVertexInput;
        FRHIVertexInput UVVertexInput;
        // std::vector<FMeshPassProcessor *> RenderPasses;

        // FBasePassMeshPassProcessor *BasePass;
        // FSkyPassMeshPassProcessor *SkyPass;

        // enum class EPass : int32
        // {
        //     BasePass,
        //     SkyPass,
        //     LightingPass,

        //     PassNum
        // };
    };
}