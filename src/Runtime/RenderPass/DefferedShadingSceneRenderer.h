#pragma once
#include "MeshBatch.h"
#include "Scene.h"
#include "Common/ContentManager.h"
#include "MeshPassProcessor.h"
#include "RHIResources.h"
#include "SceneView.h"
#include <unordered_map>
#include <vector>
#include <stack>


namespace nilou {

    DECLARE_GLOBAL_SHADER(FScreenQuadVertexShader)
    DECLARE_GLOBAL_SHADER(FRenderToScreenPixelShader)

    class FParallelMeshDrawCommands
    {
    public:
        void AddMeshDrawCommand(const FMeshDrawCommand &MeshDrawCommand);
        void Clear();
        void DispatchDraw(FDynamicRHI *RHICmdList);
    private:
        std::vector<FMeshDrawCommand> MeshCommands;
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

    struct SceneTextureCreateInfo
    {
        ivec2 OutputResolution=ivec2(1920, 1080);
        bool operator<(const SceneTextureCreateInfo& Other) const
        {
            return std::make_pair(OutputResolution.x, OutputResolution.y) < 
                   std::make_pair(Other.OutputResolution.x, Other.OutputResolution.y);
        }
    };

    class FSceneTextures
    {
    public:
        FSceneTextures(const SceneTextureCreateInfo &CreateInfo);
        ivec2 Viewport;
        RHITexture2DRef SceneColor;
        RHITexture2DRef DepthStencil;
        RHIFramebufferRef LightPassFramebuffer;
    };

    class FSceneTexturesDeffered : public FSceneTextures
    {
    public:
        FSceneTexturesDeffered(const SceneTextureCreateInfo &CreateInfo);
        RHIFramebufferRef PreZPassFramebuffer;
        RHIFramebufferRef GeometryPassFramebuffer;
        RHITexture2DRef BaseColor;
        RHITexture2DRef RelativeWorldSpacePosition;
        RHITexture2DRef WorldSpaceNormal;
        RHITexture2DRef MetallicRoughness;
        RHITexture2DRef Emissive;
        RHITexture2DRef ShadingModel;
    };


    struct ShadowMapResourceCreateInfo
    {
        ivec2 ShadowMapResolution=ivec2(2048, 2048);
        ELightType LightType;
        bool operator<(const ShadowMapResourceCreateInfo& Other) const
        {
            return std::make_tuple(ShadowMapResolution.x, ShadowMapResolution.y, LightType) < 
                   std::make_tuple(Other.ShadowMapResolution.x, Other.ShadowMapResolution.y, Other.LightType);
        }
    };

    class FShadowMapTexture
    {
    public:
        FShadowMapTexture(const ShadowMapResourceCreateInfo &CreateInfo);
        std::vector<RHIFramebufferRef> ShadowMapFramebuffers;
        RHITexture2DArrayRef DepthArray;
    };

    class FShadowMapUniformBuffer
    {
    public:

        FShadowMapUniformBuffer(const ShadowMapResourceCreateInfo &CreateInfo);

        template<int N>
        TUniformBuffer<FShadowMappingBlock<N>> *Cast()
        {
            return reinterpret_cast<TUniformBuffer<FShadowMappingBlock<N>>*>(UniformBuffer.get());
        }

        std::shared_ptr<FUniformBuffer> UniformBuffer;

        int FrustumCount;
    };
    
    class FShadowMapResource
    {
    public:
        FShadowMapResource(const ShadowMapResourceCreateInfo& CreateInfo);
        FShadowMapTexture ShadowMapTexture;
        FShadowMapUniformBuffer ShadowMapUniformBuffer;
    };


    class FViewInfo : public FSceneView
    {
    public:

        FViewInfo()
        { }

        FViewInfo(const FSceneView* View)
            : FSceneView(*View)
        { }

        FParallelMeshDrawCommands MeshDrawCommands;

        FViewElementPDI PDI;

        FSceneTextures* SceneTextures;

    };

    class FLightInfo
    {
    public:

        FLightInfo()
        { }

        FLightInfo(FLightSceneProxy* InLightSceneProxy, int32 NumViews, TUniformBufferRef<FLightShaderParameters> InLightUniformBuffer)
            : LightSceneProxy(InLightSceneProxy)
            , LightUniformBuffer(InLightUniformBuffer)
        { 
            int32 LightFrustumSize;
            switch (InLightSceneProxy->LightType) 
            {
            case ELightType::LT_Directional:
                LightFrustumSize = CASCADED_SHADOWMAP_SPLIT_COUNT;
                /** 
                 * The number of shadow maps for directional lights is 
                 * relevant to the number of views, so NumViews is required. 
                 */
                break;
            case ELightType::LT_Point:
                LightFrustumSize = 6;
                NumViews = 1;
                break;
            case ELightType::LT_Spot:
                LightFrustumSize = 1;
                NumViews = 1;
                break;
                /** 
                 * The number of shadow maps for spot/point lights is 
                 * irrelevant to the number of views, so NumViews is 1. 
                 */
            default:
                return;
            }
            ShadowMapMeshDrawCommands = std::vector<FShadowMapMeshDrawCommands>(NumViews, FShadowMapMeshDrawCommands(LightFrustumSize));
            ShadowMapMeshBatches = std::vector<FShadowMapMeshBatches>(NumViews, FShadowMapMeshBatches(LightFrustumSize));
            ShadowMapResources = std::vector<FShadowMapResource*>(NumViews);
        }

        std::vector<FShadowMapMeshDrawCommands> ShadowMapMeshDrawCommands;

        std::vector<FShadowMapMeshBatches> ShadowMapMeshBatches;

        std::vector<FShadowMapResource*> ShadowMapResources;

        TUniformBufferRef<FLightShaderParameters> LightUniformBuffer;

        FLightSceneProxy* LightSceneProxy;

    };

    class FSceneRenderer
    {
    public:

        FSceneRenderer(FSceneViewFamily* InViewFamily);

        /** The scene being rendered. */
        FScene* Scene;

        /** The view family being rendered.  This references the Views array. */
        FSceneViewFamily ViewFamily;

        /** The views being rendered. */
        std::vector<FViewInfo> Views;

        /** The lights being rendered. */
        std::vector<FLightInfo> Lights;

        virtual void Render() = 0;

        static FSceneRenderer *CreateSceneRenderer(FSceneViewFamily* ViewFamily);

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

        static FScreenQuadPositionVertexBuffer PositionVertexBuffer;
        static FScreenQuadUVVertexBuffer UVVertexBuffer;
        static FRHIVertexInput PositionVertexInput;
        static FRHIVertexInput UVVertexInput;

    protected:

        template <typename TResource, typename TCreateInfo>
        class TResourcesPool
        {
        public:

            /** Allocate a resource with given CreateInfo */
            TResource* Alloc(const TCreateInfo& CreateInfo)
            {
                auto iter = FreeResourcesMap.find(CreateInfo);
                TResource* Resource;
                if (iter != FreeResourcesMap.end() && !iter->second.empty())
                {
                    auto& stk = iter->second;
                    Resource = stk.top(); stk.pop();
                    OccupiedResourcesMap[Resource] = CreateInfo;
                    // FreeResourcesMap.erase(iter);
                }
                else 
                {
                    Resource = new TResource(CreateInfo);
                    OccupiedResourcesMap[Resource] = CreateInfo;
                }
                return Resource;
            }

            void FreeAll()
            {
                for (auto &[Resource, CreateInfo] : OccupiedResourcesMap)
                    Free(Resource);
            }

            /** Return the given resource to the pool */
            void Free(TResource* Resource)
            {
                const TCreateInfo& CreateInfo = OccupiedResourcesMap[Resource];
                FreeResourcesMap[CreateInfo].push(Resource);
                // FreeResourcesMap.insert({CreateInfo, Resource});
                OccupiedResourcesMap.erase(Resource);
            }

            /** Release all free SceneTextures */
            void ReleaseUnusedTextures()
            {
                for (auto iter = FreeResourcesMap.begin(); iter != FreeResourcesMap.end(); iter++)
                {
                    // delete iter->second;
                    auto& stk = iter->second;
                    for (int i = 0; i < stk.size(); i++)
                    {
                        TResource* res = stk.top();
                        delete res;
                        stk.pop();
                    }
                }
                FreeResourcesMap.clear();
            }

        private:
            std::map<TResource*, TCreateInfo> OccupiedResourcesMap;

            std::map<TCreateInfo, std::stack<TResource*>> FreeResourcesMap;
        };

        static TResourcesPool<FShadowMapResource, ShadowMapResourceCreateInfo> ShadowMapResourcesPool;

        static TResourcesPool<FSceneTexturesDeffered, SceneTextureCreateInfo> SceneTexturesPool;

    };

    class FDefferedShadingSceneRenderer : public FSceneRenderer
    {
    public:

        FDefferedShadingSceneRenderer(FSceneViewFamily* ViewFamily);

        virtual void Render() override;

    private:

        void InitViews(FScene *Scene);

        void ComputeViewVisibility(FScene *Scene, const std::vector<FSceneView*> &SceneViews);
        
        void RenderPreZPass(FDynamicRHI *RHICmdList);
        
        void RenderCSMShadowPass(FDynamicRHI *RHICmdList);

        void RenderBasePass(FDynamicRHI *RHICmdList);

        void RenderLightingPass(FDynamicRHI *RHICmdList);

        void RenderViewElementPass(FDynamicRHI *RHICmdList);

        void RenderToScreen(FDynamicRHI *RHICmdList);

        void UpdateReflectionProbeFactors();

    };

    extern FDefferedShadingSceneRenderer *Renderer;
}