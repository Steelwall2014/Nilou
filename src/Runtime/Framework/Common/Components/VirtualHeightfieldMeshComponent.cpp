#include <half/half.hpp>
#include "VirtualHeightfieldMeshComponent.h"
#include "BaseApplication.h"
#include "StaticMeshResources.h"
#include "DynamicMeshResources.h"
#include "PrimitiveUtils.h"
#include "Material.h"
#include "VirtualTexture2D.h"
#include "PipelineStateCache.h"

#include <glad/glad.h>

namespace nilou {
    
    DECLARE_GLOBAL_SHADER(FVHMBuildNormalTangentTextureShader)
    IMPLEMENT_SHADER_TYPE(FVHMBuildNormalTangentTextureShader, "/Shaders/VirtualHeightfieldMesh/VHM_build_normal.comp", EShaderFrequency::SF_Compute, Global)
    
    DECLARE_GLOBAL_SHADER(FVHMCreateLodTextureShader)
    IMPLEMENT_SHADER_TYPE(FVHMCreateLodTextureShader, "/Shaders/VirtualHeightfieldMesh/VHM_create_lod_texture.comp", EShaderFrequency::SF_Compute, Global)

    class FVHMCreateMinMaxShader : public FGlobalShader
	{
	public:
        DECLARE_SHADER_TYPE()
        class FDimensionForPatchMinMax : SHADER_PERMUTATION_BOOL("FOR_PATCH_MINMAX");
        using FPermutationDomain = TShaderPermutationDomain<FDimensionForPatchMinMax>;
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameter, FShaderCompilerEnvironment& Environment)
        {
            FPermutationDomain Domain(Parameter.PermutationId);
            Domain.ModifyCompilationEnvironment(Environment);
        }
	};
    IMPLEMENT_SHADER_TYPE(FVHMCreateMinMaxShader, "/Shaders/VirtualHeightfieldMesh/VHM_create_minmax.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FVHMCreateMinMaxFirstPassShader)
    IMPLEMENT_SHADER_TYPE(FVHMCreateMinMaxFirstPassShader, "/Shaders/VirtualHeightfieldMesh/VHM_create_minmax_first.comp", EShaderFrequency::SF_Compute, Global)
    
    DECLARE_GLOBAL_SHADER(FVHMCreateNodeListShader)
    IMPLEMENT_SHADER_TYPE(FVHMCreateNodeListShader, "/Shaders/VirtualHeightfieldMesh/VHM_create_nodelist.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FVHMCreatePatchShader)
    IMPLEMENT_SHADER_TYPE(FVHMCreatePatchShader, "/Shaders/VirtualHeightfieldMesh/VHM_create_patch.comp", EShaderFrequency::SF_Compute, Global)

    IMPLEMENT_VERTEX_FACTORY_TYPE(FVHMVertexFactory, "/Shaders/VertexFactories/VirtualHeightfieldMeshVertexFactory.glsl")
    
    struct WorldLodParam
    {
        vec2 NodeMeterSize;
        uvec2 NodeSideNum;
        uint32 NodeDescriptionIndexOffset;	// 用来把一个node id(i, j, lod)转换到一维
    };
    struct RenderPatch
    {
        uvec4 DeltaLod;
        vec2 Offset;
        uint32 Lod;
        RenderPatch() : DeltaLod{0, 0, 0, 0}, Offset { 0, 0 }, Lod(0) {}
    };

    static unsigned int fromNodeLoctoNodeDescriptionIndex(uvec2 nodeLoc, unsigned int lod, const WorldLodParam &param)
    {
        return param.NodeDescriptionIndexOffset + nodeLoc.x * param.NodeSideNum.y + nodeLoc.y;
    }
    static vec3 GetNodePositionWS(uvec2 nodeLoc, const WorldLodParam &Param)
    {
        // 这里没有实现从显存读取高度图，所以z是0
        vec2 xy_center_coord = (vec2(nodeLoc) + vec2(0.5)) * Param.NodeMeterSize;
        return vec3(xy_center_coord, 0);
    }
    static bool MeetScreenSize(const WorldLodParam &Param, vec3 cameraPos, uvec2 nodeLoc) {
        vec3 positionWS = GetNodePositionWS(nodeLoc, Param);
        float dis = glm::distance(cameraPos, positionWS);
        float nodeSize = std::max(Param.NodeMeterSize.x, Param.NodeMeterSize.y);
        float f = dis / (nodeSize * 1.5);
        if (f < 1)
            return false;
        return true;
    }

    void FVHMVertexFactory::SetData(const FDataType &InData)
    {
        Data = InData;
    }

    void FVHMVertexFactory::InitVertexFactory()
    {
        Elements.clear();
        if (Data.PositionComponent.VertexBuffer != nullptr)
        {
            Elements.push_back(AccessStreamComponent(Data.PositionComponent, 0, Streams));
        }
        if (Data.ColorComponent.VertexBuffer != nullptr)
        {
            Elements.push_back(AccessStreamComponent(Data.ColorComponent, 3, Streams));
        }
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
            {
                Elements.push_back(AccessStreamComponent(Data.TexCoordComponent[i], 4+i, Streams));
            }
        }
        ENQUEUE_RENDER_COMMAND(FStaticVertexFactory_InitVertexFactory)(
            [this](FDynamicRHI* RHICmdList) 
            {
                Declaration = FPipelineStateCache::GetOrCreateVertexDeclaration(Elements);
            });
    }

    bool FVHMVertexFactory::ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters)
    {
        return true;
    }

    void FVHMVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    { 
    }

    class FVirtualHeightfieldMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FVirtualHeightfieldMeshSceneProxy(UVirtualHeightfieldMeshComponent *Component)
            : FPrimitiveSceneProxy(Component)
            , NodeCount(Component->NodeCount)
            , NumPatchesPerNode(Component->NumPatchesPerNode)
            , NumQuadsPerPatch(Component->NumQuadsPerPatch)
            , NumMaxRenderingNodes(Component->NumMaxRenderingNodes)
        {
            if (Component->Material)
                Material = Component->Material->GetRenderProxy();
            if (Component->HeightfieldTexture)
            {
                HeightField = Component->HeightfieldTexture;
                HeightFieldSampler = Component->HeightfieldTexture->GetResource()->GetSamplerRHI();
            }
            if (Material == nullptr || HeightField == nullptr)
                return;
            NodesFinalFeedback = std::make_unique<uvec4[]>(Component->NumMaxRenderingNodes);
            uvec2 temp_NodeCount = NodeCount;
            while (temp_NodeCount.x % 2 == 0 && temp_NodeCount.y % 2 == 0)
            {
                LodCount++;
                temp_NodeCount.x /= 2;
                temp_NodeCount.y /= 2;
            }
            LodParams.resize(LodCount);
            vec2 Lod0NodeMeterSize = vec2(Component->NumPatchesPerNode) * vec2(Component->NumQuadsPerPatch);
            vec2 LandscapeMeterSize = Lod0NodeMeterSize * vec2(Component->NodeCount); 

            NumAllNodes = 0;
            for (int lod = 0; lod < LodCount; lod++)
            {
                WorldLodParam param;
                param.NodeMeterSize = Lod0NodeMeterSize * vec2(glm::pow(2, lod));
                param.NodeSideNum = Component->NodeCount / uvec2(glm::pow(2, lod));
                LodParams[lod] = param;
                NumAllNodes += param.NodeSideNum.x * param.NodeSideNum.y;
            }

            LodParams[0].NodeDescriptionIndexOffset = 0;
            for (int lod = 1; lod < LodCount; lod++)
            {
                LodParams[lod].NodeDescriptionIndexOffset = 
                    LodParams[lod-1].NodeDescriptionIndexOffset + 
                    LodParams[lod-1].NodeSideNum.x * LodParams[lod-1].NodeSideNum.y;
            }

            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildFlatSurfaceVerts(uvec2(NumQuadsPerPatch+1), OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);
            BeginInitResource(&IndexBuffer);

            VertexBuffers.Positions.Init(OutVerts.size());
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                VertexBuffers.TexCoords[i].Init(OutVerts.size());
            VertexBuffers.Colors.Init(OutVerts.size());
            
            for (int VertexIndex = 0; VertexIndex < OutVerts.size(); VertexIndex++)
            {
                const FDynamicMeshVertex &Vertex = OutVerts[VertexIndex];
                VertexBuffers.Positions.SetVertexValue(VertexIndex, Vertex.Position);
                for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                    VertexBuffers.TexCoords[i].SetVertexValue(VertexIndex, Vertex.TextureCoordinate[i]);
                VertexBuffers.Colors.SetVertexValue(VertexIndex, vec4(Vertex.Color, 1));
            }

            {
                BeginInitResource(&VertexBuffers.Positions);
                for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                    BeginInitResource(&VertexBuffers.TexCoords[i]);
                BeginInitResource(&VertexBuffers.Colors);
                
                FVHMVertexFactory::FDataType Data;
                VertexBuffers.Positions.BindToVertexFactoryData(Data.PositionComponent);
                for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                    VertexBuffers.TexCoords[i].BindToVertexFactoryData(Data.TexCoordComponent[i]);
                VertexBuffers.Colors.BindToVertexFactoryData(Data.ColorComponent);
                VertexFactory.SetData(Data);
                VertexFactory.InitVertexFactory();
            }

            // PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FVirtualHeightfieldMeshSceneProxy::PreRenderCallBack);
            // PreRenderHandle = GetAppication()->GetPostRenderDelegate().Add(this, &FVirtualHeightfieldMeshSceneProxy::PostRenderCallBack);

            ENQUEUE_RENDER_COMMAND(FVirtualHeightfieldMeshSceneProxy_Constructor)(
                [this](FDynamicRHI *RHICmdList)
                {

                    LodTexture = RHICmdList->RHICreateTexture2D(
                        "LodTexture", 
                        EPixelFormat::PF_R16F, 
                        1, 
                        NodeCount.x, 
                        NodeCount.y, TexCreate_UAV);
                    uvec2 size = LodParams[0].NodeSideNum * uvec2(8);
                    HeightMinMaxTexture = RHICmdList->RHICreateTexture2D(
                            "HeightMinMaxTexture", 
                            EPixelFormat::PF_R16G16F, 
                            LodCount+3, 
                            size.x, 
                            size.y, TexCreate_UAV);
                    RHITextureParams params;
                    params.Mag_Filter = ETextureFilters::TF_Nearest;
                    params.Min_Filter = ETextureFilters::TF_Nearest;
                    HeightMinMaxSampler = FRHISampler(HeightMinMaxTexture.get(), TStaticSamplerState<TF_Nearest, TF_Nearest>::CreateRHI());
                    for (int i = 0; i < LodCount+3; i++)
                    {
                        HeightMinMaxTextureViews.push_back(RHICmdList->RHICreateTextureView2D(
                            HeightMinMaxTexture.get(),
                            EPixelFormat::PF_R16G16F, 
                            i, 1, 0));
                    }

                    LodParamsBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        LodParams.size() * sizeof(WorldLodParam), 
                        LodParams.data());
                    NodeDescriptionBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NumAllNodes * 4, 
                        0);
                    FinalNodeListBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NumMaxRenderingNodes*sizeof(uvec4), nullptr);
                    NodeIDs_TempA = RHICmdList->RHICreateShaderStorageBuffer(
                        NumMaxRenderingNodes*sizeof(uvec2), nullptr);
                    NodeIDs_TempB = RHICmdList->RHICreateShaderStorageBuffer(
                        NumMaxRenderingNodes*sizeof(uvec2), nullptr);

                    FinalNodeListIndirectArgs = RHICmdList->RHICreateDispatchIndirectBuffer(0, 1, 1);
                    PatchListBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NumMaxRenderingNodes*NumPatchesPerNode*NumPatchesPerNode*sizeof(RenderPatch), 
                        nullptr);

                    CreateNodeListBlock = CreateUniformBuffer<FCreateNodeListBlock>();
                    CreateNodeListBlock->Data.MaxLOD = LodCount-1;
                    CreateNodeListBlock->InitResource();

                    QuadTreeParameters = CreateUniformBuffer<FQuadTreeParameters>();
                    QuadTreeParameters->Data.NumQuadsPerPatch = NumQuadsPerPatch;
                    QuadTreeParameters->Data.NumPatchesPerNode = NumPatchesPerNode;
                    QuadTreeParameters->Data.NodeCount = NodeCount;
                    QuadTreeParameters->Data.LODNum = LodCount;
                    QuadTreeParameters->Data.NumHeightfieldTextureMipmap = HeightField->GetResource()->NumMips;
                    QuadTreeParameters->InitResource();

                    CreatePatchBlock = CreateUniformBuffer<FCreatePatchBlock>();
                    CreatePatchBlock->Data.LodTextureSize = NodeCount;
                    CreatePatchBlock->InitResource();

                    BuildNormalTangentBlock = CreateUniformBuffer<FBuildNormalTangentBlock>();
                    BuildNormalTangentBlock->Data.HeightfieldWidth = HeightFieldSampler->Texture->GetSizeXYZ().x;
                    BuildNormalTangentBlock->Data.HeightfieldHeight = HeightFieldSampler->Texture->GetSizeXYZ().y;
                    BuildNormalTangentBlock->Data.PixelMeterSize = vec2(NumQuadsPerPatch * NumPatchesPerNode * NodeCount) / vec2(HeightFieldSampler->Texture->GetSizeXYZ());
                    BuildNormalTangentBlock->InitResource();

                    DrawIndirectArgs = RHICmdList->RHICreateDrawElementsIndirectBuffer(IndexBuffer.GetNumIndices(), 0, 0, 0, 0);
                    
                    BuildMinMaxTexture();

                    // BuildNormalTexture();
                });
        }

        void GenerateRenderPatches(FDynamicRHI* RHICmdList, TUniformBuffer<FViewShaderParameters>* ViewShaderParameters)
        {
            // FViewSceneInfo *ViewInfo = GetAppication()->GetWorld()->MainCameraComponent->GetSceneProxy()->GetViewSceneInfo();
            CreateNodeListGPU(ViewShaderParameters);
            CreateLodTexture();
            CreatePatch(ViewShaderParameters);

            uint32* final_nodelist_size = (uint32*)RHICmdList->RHILockBuffer(FinalNodeListIndirectArgs.get(), 0, sizeof(uint32), RLM_ReadOnly);
            uint32 NodesFinalFeedbackSize = *final_nodelist_size;
            RHICmdList->RHIUnlockBuffer(FinalNodeListIndirectArgs.get());
            
            uvec4* data = (uvec4*)RHICmdList->RHILockBuffer(FinalNodeListBuffer.get(), 0, NodesFinalFeedbackSize * sizeof(uvec4), RLM_ReadOnly);
            memcpy(NodesFinalFeedback.get(), data, NodesFinalFeedbackSize * sizeof(uvec4));
            RHICmdList->RHIUnlockBuffer(FinalNodeListBuffer.get());

            vec2 HeightTextureMeterSize = NumQuadsPerPatch * NumPatchesPerNode * NodeCount;
            for (int i = 0; i < NodesFinalFeedbackSize; i++)
            {
                uvec4 Node = NodesFinalFeedback[i];
                uint32 Lod = Node.z;
                vec2 node_min = vec2(Node) * LodParams[Lod].NodeMeterSize;
                vec2 node_max = (vec2(Node)+vec2(1)) * LodParams[Lod].NodeMeterSize;

                // For simplicity, we just use the LOD level of quad-tree as the mipmap level
                int MipmapLevel = Lod;
                HeightField->UpdateBound(node_min / HeightTextureMeterSize, node_max / HeightTextureMeterSize, MipmapLevel);
            }
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            if (Material == nullptr || HeightField == nullptr)
                return;
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    if (Views[ViewIndex]->ViewUniformBuffer == nullptr)
                        continue;
                    GenerateRenderPatches(FDynamicRHI::GetDynamicRHI(), Views[ViewIndex]->ViewUniformBuffer.get());
                    FMeshBatch Mesh;
                    Mesh.CastShadow = bCastShadow;
                    Mesh.MaterialRenderProxy = Material;
                    Mesh.Element.IndexBuffer = &IndexBuffer;
                    Mesh.Element.VertexFactory = &VertexFactory;
                    Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
                    Mesh.Element.Bindings.SetElementShaderBinding("Patch_Buffer", PatchListBuffer.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("FQuadTreeParameters", QuadTreeParameters->GetRHI());
                    Mesh.Element.Bindings.SetElementShaderBinding("HeightfieldTexture", HeightFieldSampler);
                    // Mesh.Element.Bindings.SetElementShaderBinding("MinMaxMap", &HeightMinMaxSampler);
                    Mesh.Element.Bindings.SetElementShaderBinding("FBuildNormalTangentBlock", BuildNormalTangentBlock->GetRHI());

                    Mesh.Element.NumVertices = 0;
                    Mesh.Element.IndirectArgsBuffer = DrawIndirectArgs.get();
                    Mesh.Element.IndirectArgsOffset = 0;

                    Collector.AddMesh(ViewIndex, Mesh);
                }
            }
        }

        void BuildMinMaxTexture()
        {
            constexpr int BUILD_MINMAX_LOCAL_SIZE = 32;
            auto BuildMinMaxBlock = CreateUniformBuffer<FBuildMinMaxBlock>();
            BuildMinMaxBlock->InitResource();

            FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxFirstPassShader::StaticType, 0);
            FShaderInstance *CreateMinMaxFirstPassShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxFirstPassShader->GetComputeShaderRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FQuadTreeParameters", QuadTreeParameters->GetRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute, 
                "HeightMap", *HeightFieldSampler);
            FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "OutMinMaxMap", HeightMinMaxTextureViews[0].get(), EDataAccessFlag::DA_WriteOnly);
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FBuildMinMaxBlock", BuildMinMaxBlock->GetRHI());

            uint32 MaxPhysicalMemoryByte = HeightField->MaxPhysicalMemoryByte;
            uint32 BytePerTile = HeightField->GetBytePerTile();
            int32 TileNumLimit = MaxPhysicalMemoryByte / BytePerTile;
            uvec2 NumTiles = HeightField->GetNumTiles();
            if (NumTiles.x * NumTiles.y < TileNumLimit) // The physical memory can hold all of the texture.
            {
                BuildMinMaxBlock->Data.Offset = uvec2(0, 0);
                BuildMinMaxBlock->UpdateUniformBuffer();
                HeightField->UpdateBoundSync(vec2(0), vec2(1), 0);
                int group_num_y = HeightMinMaxTexture->GetSizeY() / BUILD_MINMAX_LOCAL_SIZE;
                int group_num_x = HeightMinMaxTexture->GetSizeX() / BUILD_MINMAX_LOCAL_SIZE;
                FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num_x, group_num_y, 1);
            }
            else // The physical memory can not hold all of the texture.
            {
                uint32 NumRowsPerDispatch = TileNumLimit / NumTiles.x;
                if (NumRowsPerDispatch == 0)    // The physical memory can't even hold a row of tiles of the texture.
                {
                    int DispatchCountPerRow = glm::ceil(float(NumTiles.x) / float(TileNumLimit));
                    for (int Row = 0; Row < NumTiles.y; Row++)
                    {
                        float Row_Min = float(Row) / NumTiles.y;
                        float Row_Max = float(Row+1) / NumTiles.y;
                        for (int i = 0; i < DispatchCountPerRow; i++)
                        {
                            float Col_Min = float(i) * NumTiles.x/DispatchCountPerRow / NumTiles.x;
                            float Col_Max = float(i+1) * NumTiles.x/DispatchCountPerRow / NumTiles.x;
                            HeightField->UpdateBoundSync(vec2(Col_Min, Row_Min), vec2(Col_Max, Row_Max), 0);
                            int group_num_y = HeightMinMaxTexture->GetSizeY() * (Row_Max - Row_Min) / BUILD_MINMAX_LOCAL_SIZE;
                            int group_num_x = HeightMinMaxTexture->GetSizeX() * (Col_Max - Col_Min) / BUILD_MINMAX_LOCAL_SIZE;
                            BuildMinMaxBlock->Data.Offset = uvec2(HeightMinMaxTexture->GetSizeX() * Col_Min, HeightMinMaxTexture->GetSizeY() * Row_Min);
                            BuildMinMaxBlock->UpdateUniformBuffer();
                            FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num_x, group_num_y, 1);
                        }
                    }
                }
                else    // The physical memory can hold several rows of tiles of the texture.
                {
                    int DispatchCount = glm::ceil(float(NumTiles.y) / float(NumRowsPerDispatch));
                    for (int i = 0; i < DispatchCount; i++)
                    {
                        float Row_Min = glm::clamp(float(i) * NumRowsPerDispatch / NumTiles.y, 0.f, 1.f);
                        float Row_Max = glm::clamp(float(i+1) * NumRowsPerDispatch / NumTiles.y, 0.f, 1.f);
                        HeightField->UpdateBoundSync(vec2(0, Row_Min), vec2(1, Row_Max), 0);
                        int group_num_y = HeightMinMaxTexture->GetSizeY() * (Row_Max - Row_Min) / BUILD_MINMAX_LOCAL_SIZE;
                        int group_num_x = HeightMinMaxTexture->GetSizeX() / BUILD_MINMAX_LOCAL_SIZE;
                        BuildMinMaxBlock->Data.Offset = uvec2(0, HeightMinMaxTexture->GetSizeY() * Row_Min);
                        BuildMinMaxBlock->UpdateUniformBuffer();
                        FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num_x, group_num_y, 1);
                    }
                }
            }

            FVHMCreateMinMaxShader::FPermutationDomain PermutationVector;
            PermutationVector.Set<FVHMCreateMinMaxShader::FDimensionForPatchMinMax>(true);
            {   // Create MinMax texture for patches
                FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxShader::StaticType, PermutationVector.ToDimensionValueId());
                FShaderInstance *CreateMinMaxShader = GetGlobalShader(PermutationParameters);
                FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxShader->GetComputeShaderRHI());
                for (int LOD = 1; LOD < LodCount; LOD++)
                {
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "InMinMaxMap", HeightMinMaxTextureViews[LOD - 1].get(), EDataAccessFlag::DA_ReadOnly);
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "OutMinMaxMap", HeightMinMaxTextureViews[LOD].get(), EDataAccessFlag::DA_WriteOnly);
                    uvec2 group_num = LodParams[LOD].NodeSideNum;
                    FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);
                }
            }
            PermutationVector.Set<FVHMCreateMinMaxShader::FDimensionForPatchMinMax>(false);
            {   // Create MinMax texture for nodes
                FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxShader::StaticType, PermutationVector.ToDimensionValueId());
                FShaderInstance *CreateMinMaxShader = GetGlobalShader(PermutationParameters);
                FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxShader->GetComputeShaderRHI());
                for (int LOD = LodCount; LOD < LodCount+3; LOD++)
                {
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "InMinMaxMap", HeightMinMaxTextureViews[LOD - 1].get(), EDataAccessFlag::DA_ReadOnly);
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "OutMinMaxMap", HeightMinMaxTextureViews[LOD].get(), EDataAccessFlag::DA_WriteOnly);
                    uvec2 group_num = LodParams[LOD-3].NodeSideNum;
                    FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);
                }
            }
        }

        void CreateNodeListGPU(TUniformBuffer<FViewShaderParameters> *ViewShaderParameters)
        {
            static uint32 zero_value = 0;
            CreateNodeListBlock->Data.ScreenSizeDenominator = 2 * glm::tan(0.5*ViewShaderParameters->Data.CameraVerticalFieldOfView);
            RHIGetError();
            uint32 index_b_value = LodParams[LodCount-1].NodeSideNum.x * LodParams[LodCount-1].NodeSideNum.y;
            RHIBufferRef IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(4, 4, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, &zero_value);
            RHIBufferRef IndexFinal = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(4, 4, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, &zero_value);

            RHIGetError();
            RHIBufferRef indirectArgs = FDynamicRHI::GetDynamicRHI()->RHICreateDispatchIndirectBuffer(index_b_value, 1, 1);

            RHIGetError();
            FShaderPermutationParameters PermutationParameters(&FVHMCreateNodeListShader::StaticType, 0);
            FShaderInstance *CreateNodeListShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateNodeListShader->GetComputeShaderRHI());
            
            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "NodeIDs_Final_Buffer", FinalNodeListBuffer.get());
            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "index_Final", IndexFinal.get());
            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "LODParams_Buffer", LodParamsBuffer.get());
            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "NodeDescription_Buffer", NodeDescriptionBuffer.get());
            
            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute, 
                "MinMaxMap", HeightMinMaxSampler);
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FCreateNodeListBlock", CreateNodeListBlock->GetRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FViewShaderParameters", ViewShaderParameters->GetRHI());
            
            RHIGetError();
            for (int lod = LodCount - 1; lod >= 0; lod--)
            {
                CreateNodeListBlock->Data.PassLOD = lod;
                CreateNodeListBlock->UpdateUniformBuffer();
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "NodeIDs_TempA_Buffer", NodeIDs_TempA.get());
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "NodeIDs_TempB_Buffer", NodeIDs_TempB.get());
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "index_B", IndexB.get());

                RHIGetError();
                FDynamicRHI::GetDynamicRHI()->RHIDispatchIndirect(indirectArgs.get());

                RHIGetError();
                FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexB, indirectArgs, 0, 0, 4);
                

                RHIGetError();
                std::swap(NodeIDs_TempA, NodeIDs_TempB);
                IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(4, 4, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, &zero_value);
            }

            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexFinal, FinalNodeListIndirectArgs, 0, 0, 4);
            
        }

        void CreateLodTexture()
        {
            FShaderPermutationParameters PermutationParameters(&FVHMCreateLodTextureShader::StaticType, 0);
            FShaderInstance *CreateLodTextureShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateLodTextureShader->GetComputeShaderRHI());

            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "NodeDescription_Buffer", NodeDescriptionBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "LODParams_Buffer", LodParamsBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "LODMap", LodTexture.get(), EDataAccessFlag::DA_WriteOnly);
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FQuadTreeParameters", QuadTreeParameters->GetRHI());

            uvec2 group_num = NodeCount / uvec2(32);
            FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);

        }

        void CreatePatch(TUniformBuffer<FViewShaderParameters> *ViewShaderParameters)
        {
            static uint32 zero_value = 0;
            AtomicPatchCounterBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateBuffer(4, 4, EBufferUsageFlags::StructuredBuffer | EBufferUsageFlags::Dynamic, &zero_value);

            FShaderPermutationParameters PermutationParameters(&FVHMCreatePatchShader::StaticType, 0);
            FShaderInstance *CreatePatchShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreatePatchShader->GetComputeShaderRHI());

            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "NodeIDs_Final_Buffer", FinalNodeListBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "Patch_Buffer", PatchListBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "LODParams_Buffer", LodParamsBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "patch_index", AtomicPatchCounterBuffer.get());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "LODMap", LodTexture.get(), EDataAccessFlag::DA_ReadOnly);
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FCreatePatchBlock", CreatePatchBlock->GetRHI());

            FDynamicRHI::GetDynamicRHI()->RHIDispatchIndirect(FinalNodeListIndirectArgs.get());
            FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(AtomicPatchCounterBuffer, DrawIndirectArgs, 0, 4, 4);
            
            
        }

        virtual void DestroyRenderThreadResources() override
        {
            VertexBuffers.ReleaseResource();
            IndexBuffer.ReleaseResource();
            if (CreateNodeListBlock)
                CreateNodeListBlock->ReleaseResource();
            if (QuadTreeParameters)
                QuadTreeParameters->ReleaseResource();
            if (CreatePatchBlock)
                CreatePatchBlock->ReleaseResource();
            if (BuildNormalTangentBlock)
                BuildNormalTangentBlock->ReleaseResource();
            FPrimitiveSceneProxy::DestroyRenderThreadResources();
        }

	    FMaterialRenderProxy* Material = nullptr;
        UVirtualTexture* HeightField = nullptr;
        FRHISampler* HeightFieldSampler = nullptr;
        RHITexture2DRef HeightMinMaxTexture;
        FRHISampler HeightMinMaxSampler;
        std::vector<RHITexture2DRef> HeightMinMaxTextureViews;
	    RHITexture2DRef LodTexture;

		RHIBufferRef LodParamsBuffer;
		RHIBufferRef NodeDescriptionBuffer;
		RHIBufferRef NodeIDs_TempA;
		RHIBufferRef NodeIDs_TempB;
		RHIBufferRef FinalNodeListBuffer;
		RHIBufferRef FinalNodeListIndirectArgs;
		RHIBufferRef PatchListBuffer;
		RHIBufferRef AtomicPatchCounterBuffer;
        RHIBufferRef DrawIndirectArgs;

        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FVHMVertexFactory VertexFactory;

        TUniformBufferRef<FCreateNodeListBlock> CreateNodeListBlock;
        TUniformBufferRef<FQuadTreeParameters> QuadTreeParameters;
        TUniformBufferRef<FCreatePatchBlock> CreatePatchBlock;
        TUniformBufferRef<FBuildNormalTangentBlock> BuildNormalTangentBlock;

        std::vector<WorldLodParam> LodParams;
		uint32 NumMaxRenderingNodes;
        uint32 NumAllNodes;

        uint32 NumQuadsPerPatch;
        uint32 NumPatchesPerNode;
        uvec2 NodeCount;
        
        uint8 LodCount = 1;
        FDelegateHandle PreRenderHandle;
        FDelegateHandle PostRenderHandle;

        std::unique_ptr<uvec4[]> NodesFinalFeedback;

    };



    UVirtualHeightfieldMeshComponent::UVirtualHeightfieldMeshComponent()
    {
        // bCastShadow = false;
    }

    FBoundingBox UVirtualHeightfieldMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
    {
        if (SceneProxy)
        {
            FVirtualHeightfieldMeshSceneProxy *Proxy = static_cast<FVirtualHeightfieldMeshSceneProxy*>(SceneProxy);
            vec2 ExtentXY = vec2(NodeCount) * vec2(NumPatchesPerNode) * vec2(NumQuadsPerPatch);
            vec3 BoxMax = vec3(ExtentXY, HeightfieldMax);
            vec3 BoxMin = vec3(0, 0, HeightfieldMin);
            return FBoundingBox(BoxMin, BoxMax).TransformBy(LocalToWorld);
        }
        return FBoundingBox(vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f)).TransformBy(LocalToWorld);
    }

    FPrimitiveSceneProxy *UVirtualHeightfieldMeshComponent::CreateSceneProxy()
    {
        return new FVirtualHeightfieldMeshSceneProxy(this);
    }


}