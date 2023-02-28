#pragma once

#include "SkyAtmospherePassRendering.h"

#include "Common/MeshBatch.h"
#include "Common/Scene.h"
#include "Common/ContentManager.h"
#include "MeshPassProcessor.h"
#include "RHIResources.h"
#include "SceneView.h"
#include <unordered_map>
#include <vector>


namespace nilou {

    DECLARE_GLOBAL_SHADER(FScreenQuadVertexShader)
    DECLARE_GLOBAL_SHADER(FRenderToScreenPixelShader)

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
        RHIFramebufferRef PreZPassFrameBuffer;
        RHIFramebufferRef GeometryPassFrameBuffer;
        RHIFramebufferRef FrameBuffer;
        RHITexture2DRef BaseColor;
        RHITexture2DRef RelativeWorldSpacePosition;
        RHITexture2DRef WorldSpaceNormal;
        RHITexture2DRef MetallicRoughness;
        RHITexture2DRef Emissive;
        RHITexture2DRef DepthStencil;
        RHITexture2DRef SceneColor;
        ivec2 Viewport;
        FSceneTextures(const ivec2 &ScreenResolution);
    };

    class FShadowMapTextures
    {
    public:
        std::vector<RHIFramebufferRef> FrameBuffers;
        RHITexture2DArrayRef DepthArray;
        FShadowMapTextures(const ivec2 &ShadowMapResolution, int ShadowMapArraySize);
    };

    class FShadowMapUniformBuffers
    {
    public:
    
        template<int FrustumCount>
        static FShadowMapUniformBuffers Create()
        {
            FShadowMapUniformBuffers obj;
            auto UBO = CreateUniformBuffer<FShadowMappingBlock<FrustumCount>>();
            UBO->SetUsage(EUniformBufferUsage::UniformBuffer_SingleFrame);
            obj.UniformBuffer = UBO;
            obj.FrustumCount = FrustumCount;
            UBO->InitRHI();
            return obj;
        }

        template<int FrustumCount>
        static TUniformBuffer<FShadowMappingBlock<FrustumCount>> *Cast(FShadowMapUniformBuffers &ShadowMapUniformBuffer)
        {
            if (ShadowMapUniformBuffer.FrustumCount != FrustumCount)
                return nullptr;
            return static_cast<TUniformBuffer<FShadowMappingBlock<FrustumCount>>*>(ShadowMapUniformBuffer.UniformBuffer.get());
        }

        std::shared_ptr<FUniformBuffer> UniformBuffer;
        int FrustumCount;

    private:
        FShadowMapUniformBuffers() { }
    };

    class FShadowMapMeshDrawCommands
    {
    public:
        FShadowMapMeshDrawCommands(int LightFrustumSize)
        {
            for (int i = 0; i < LightFrustumSize; i++)
                DrawCommands.push_back(FParallelMeshDrawCommands());
        }
        std::vector<FParallelMeshDrawCommands> DrawCommands;
    };

    class FShadowMapMeshBatches
    {
    public:
        FShadowMapMeshBatches(int LightFrustumSize)
        {
            for (int i = 0; i < LightFrustumSize; i++)
                MeshBatches.push_back(std::vector<FMeshBatch>());
        }
        std::vector<std::vector<FMeshBatch>> MeshBatches;
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

        void OnAddView(FViewSceneInfo *CameraInfo);
        void OnRemoveView(FViewSceneInfo *CameraInfo);
        void OnResizeView(FViewSceneInfo *CameraInfo);

        void OnAddLight(FLightSceneInfo *LightInfo);
        void OnRemoveLight(FLightSceneInfo *LightInfo);

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
                FVertexBuffer::InitRHI();
                VertexBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(sizeof(glm::vec4), sizeof(Vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, Vertices);
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
                FVertexBuffer::InitRHI();
                VertexBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(sizeof(glm::vec2), sizeof(Vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, Vertices);
            }
        private:
            glm::vec2 Vertices[4];
        };

        struct ShadowMapSplitIndexUBOs
        {
            ShadowMapSplitIndexUBOs()
            {
                for (int i = 0; i < CASCADED_SHADOWMAP_SPLIT_COUNT; i++)
                {
                    UBOs[i] = CreateUniformBuffer<FShadowMapFrustumIndex>();
                    UBOs[i]->SetUsage(EUniformBufferUsage::UniformBuffer_MultiFrame);
                    UBOs[i]->Data.FrustumIndex = i;
                    UBOs[i]->InitRHI();
                }
            }
            std::array<TUniformBufferRef<FShadowMapFrustumIndex>, CASCADED_SHADOWMAP_SPLIT_COUNT> UBOs;
        };
        ShadowMapSplitIndexUBOs SplitIndexUBO;

        // void SetupMeshPass(FSceneView &View, const std::vector<FMeshBatch> &ViewMeshBatches, std::vector<FMeshDrawCommand> &OutMeshDrawCommands);
        void InitViews(FScene *Scene);

        void ComputeViewVisibility(FScene *Scene, const std::vector<FSceneView> &SceneViews);
        
        void RenderPreZPass(FDynamicRHI *RHICmdList);
        
        void RenderCSMShadowPass(FDynamicRHI *RHICmdList);

        void RenderBasePass(FDynamicRHI *RHICmdList);

        void RenderLightingPass(FDynamicRHI *RHICmdList);

        void RenderAtmospherePass(FDynamicRHI *RHICmdList);

        void RenderViewElementPass(FDynamicRHI *RHICmdList);

        void RenderToScreen(FDynamicRHI *RHICmdList);

        FScene *Scene = nullptr;
        
        struct RenderViews
        {
            RenderViews(FViewSceneInfo *InViewSceneInfo, const FSceneTextures &InSceneTextures)
                : ViewSceneInfo(InViewSceneInfo)
                , SceneTextures(InSceneTextures)
                , PDI(InViewSceneInfo->PDI.get())
            { }
            FViewSceneInfo *ViewSceneInfo;
            FSceneTextures SceneTextures;
            FParallelMeshDrawCommands MeshDrawCommands;
            std::vector<FMeshBatch> MeshBatches;
            FViewElementPDI* PDI;
        };
        std::vector<RenderViews> Views;
        
        struct RenderLights
        {
            RenderLights(FLightSceneInfo *InLightSceneInfo)
                : LightSceneInfo(InLightSceneInfo) { }
            FLightSceneInfo *LightSceneInfo;

            // 如果是平行光，那么下面这些vector每一个元素对应一个view
            // 如果是点光源或者射灯，那么下面这些vector的长度为1
            std::vector<FShadowMapTextures> ShadowMapTextures;
            std::vector<FShadowMapUniformBuffers> ShadowMapUniformBuffers;
            std::vector<FShadowMapMeshDrawCommands> ShadowMapMeshDrawCommands;
            std::vector<FShadowMapMeshBatches> ShadowMapMeshBatches;
        };
        std::vector<RenderLights> Lights;



        // The collector used for scene rendering
        FMeshElementCollector Collector;

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

    extern FDefferedShadingSceneRenderer *Renderer;
}