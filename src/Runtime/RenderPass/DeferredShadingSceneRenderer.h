#pragma once
#include "MeshBatch.h"
#include "Scene.h"
#include "Common/ContentManager.h"
#include "MeshPassProcessor.h"
#include "RHIResources.h"
#include "SceneView.h"
#include "PipelineStateCache.h"
#include <unordered_map>
#include <vector>
#include <stack>


namespace nilou {

    DECLARE_GLOBAL_SHADER(FScreenQuadVertexShader)
	class FRenderToScreenPixelShader : public FGlobalShader
	{
	public:
        BEGIN_UNIFORM_BUFFER_STRUCT(UniformBlock)
            SHADER_PARAMETER_STRUCT(float, GammaCorrection)
            SHADER_PARAMETER_STRUCT(int, bEnableToneMapping)
        END_UNIFORM_BUFFER_STRUCT()
		DECLARE_SHADER_TYPE()
	};

    class FParallelMeshDrawCommands
    {
    public:
        void AddMeshDrawCommand(const FMeshDrawCommand &MeshDrawCommand);
        void Clear();
        void DispatchDraw(RHICommandList& RHICmdList) const;
    private:
        std::vector<FMeshDrawCommand> MeshCommands;
    };

    class FShadowMapResource
    {
    public:
        std::vector<RDGTextureView*> DepthViews;
        RDGTexture* DepthArray;
        RDGBuffer* ShadowMapUniformBuffer;
        std::vector<FShadowMappingParameters> Frustums;
    };

    class FLightInfo
    {
    public:

        std::vector<FShadowMapResource> ShadowMapResources;

        RDGBuffer* LightUniformBuffer;

        ELightType LightType;

        FLightSceneProxy* LightSceneProxy;

    };

    class FSceneRenderer
    {
    public:

        FSceneRenderer(FSceneViewFamily& InViewFamily);

        /** The scene being rendered. */
        FScene* Scene;

        /** The view family being rendered.  This references the Views array. */
        FSceneViewFamily& ViewFamily;

        /** The views being rendered. */
        std::vector<FSceneView>& Views;
        std::vector<FViewElementPDI> ViewPDIs;
        std::vector<std::vector<FMeshBatch>> ViewMeshBatches;
        std::vector<FParallelMeshDrawCommands> ViewMeshDrawCommands;

        /** The lights being rendered. */
        std::vector<FLightInfo> Lights;

        std::vector<FParallelMeshDrawCommands> MeshDrawCommands;

        virtual void Render(RenderGraph& Graph) = 0;

        static FSceneRenderer *CreateSceneRenderer(FSceneViewFamily& ViewFamily);

        class FScreenQuadPositionVertexBuffer : public FVertexBuffer
        {
        public:
            FScreenQuadPositionVertexBuffer()
            {
                Vertices[0] = vec4(-1, 1, 0, 1);
                Vertices[1] = vec4(-1, -1, 0, 1);
                Vertices[2] = vec4(1, 1, 0, 1);
                Vertices[3] = vec4(1, -1, 0, 1);
            }

        private:
            vec4 Vertices[4];
        };

        class FScreenQuadUVVertexBuffer : public FVertexBuffer
        {
        public:
            FScreenQuadUVVertexBuffer()
            {
                Vertices[0] = vec2(0, 1);
                Vertices[1] = vec2(0, 0);
                Vertices[2] = vec2(1, 1);
                Vertices[3] = vec2(1, 0);
            }
 
        private:
            vec2 Vertices[4];
        };

        static FScreenQuadPositionVertexBuffer PositionVertexBuffer;
        static FScreenQuadUVVertexBuffer UVVertexBuffer;
        static FRHIVertexDeclaration* ScreenQuadVertexDeclaration;

        // template <typename TResource, typename TCreateInfo>
        // class TResourcesPool
        // {
        // public:

        //     /** Allocate a resource with given CreateInfo */
        //     TResource* Alloc(const TCreateInfo& CreateInfo)
        //     {
        //         auto iter = FreeResourcesMap.find(CreateInfo);
        //         TResource* Resource;
        //         if (iter != FreeResourcesMap.end() && !iter->second.empty())
        //         {
        //             auto& stk = iter->second;
        //             Resource = stk.back(); stk.pop_back();
        //             OccupiedResourcesMap[Resource] = CreateInfo;
        //             // FreeResourcesMap.erase(iter);
        //         }
        //         else 
        //         {
        //             Resource = new TResource(CreateInfo);
        //             OccupiedResourcesMap[Resource] = CreateInfo;
        //         }
        //         return Resource;
        //     }

        //     void FreeAll()
        //     {
        //         for (auto &[Resource, CreateInfo] : OccupiedResourcesMap)
        //             Free(Resource);
        //     }

        //     /** Return the given resource to the pool */
        //     void Free(TResource* Resource)
        //     {
        //         const TCreateInfo& CreateInfo = OccupiedResourcesMap[Resource];
        //         FreeResourcesMap[CreateInfo].push_back(Resource);
        //         // FreeResourcesMap.insert({CreateInfo, Resource});
        //         OccupiedResourcesMap.erase(Resource);
        //     }

        //     /** Release all resources */
        //     void ReleaseAll()
        //     {
        //         assert(OccupiedResourcesMap.size() == 0);
        //         for (auto& [CreateInfo, stk] : FreeResourcesMap)
        //         {
        //             for (auto& res : stk)
        //             {
        //                 delete res;
        //             }
        //         }
        //         FreeResourcesMap.clear();
        //     }

        // private:
        //     std::map<TResource*, TCreateInfo> OccupiedResourcesMap;
        //     std::map<TCreateInfo, std::vector<TResource*>> FreeResourcesMap;
        // };

        // static TResourcesPool<FShadowMapResource, ShadowMapResourceCreateInfo> ShadowMapResourcesPool;

        // static TResourcesPool<FSceneTexturesDeferred, SceneTextureCreateInfo> SceneTexturesPool;

    };

    class FDeferredShadingSceneRenderer : public FSceneRenderer
    {
    public:

        struct FSceneTextures
        {
            ivec2 Viewport;
            RDGTexture* SceneColor;
            RDGTexture* DepthStencil;

            RDGTexture* BaseColor;
            RDGTexture* RelativeWorldSpacePosition;
            RDGTexture* WorldSpaceNormal;
            RDGTexture* MetallicRoughness;
            RDGTexture* Emissive;
            RDGTexture* ShadingModel;
        };

        FDeferredShadingSceneRenderer(FSceneViewFamily& ViewFamily);

        virtual void Render(RenderGraph& Graph) override;

    private:

        std::vector<FSceneTextures> ViewSceneTextures;

        void InitViews(RenderGraph& Graph);

        void ComputeViewVisibility(FSceneViewFamily& ViewFamily, std::vector<std::vector<FMeshBatch>>& OutViewMeshBatches, std::vector<FViewElementPDI>& OutViewPDIs);
        
        void RenderPreZPass(RenderGraph& Graph);
        
        void RenderCSMShadowPass(RenderGraph& Graph);

        void RenderBasePass(RenderGraph& Graph);

        void RenderIndirectLightingPass(RenderGraph& Graph);

        void RenderLightingPass(RenderGraph& Graph);

        void RenderViewElementPass(RenderGraph& Graph);

        void RenderToScreen(RenderGraph& Graph);

        void UpdateReflectionProbeFactors();

    };

    BEGIN_UNIFORM_BUFFER_STRUCT(BasePassPixelShaderUniformBlock)
        SHADER_PARAMETER(uint32, MaterialShadingModel)
        SHADER_PARAMETER(uint32, PrefilterEnvTextureNumMips)
        SHADER_PARAMETER(float, ReflectionProbeFactor)
    END_UNIFORM_BUFFER_STRUCT()

    extern FDeferredShadingSceneRenderer *Renderer;
}