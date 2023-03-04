#include <half/half.hpp>
#include "VirtualHeightfieldMeshComponent.h"
#include "Common/BaseApplication.h"
#include "Common/StaticMeshResources.h"
#include "Common/DynamicMeshResources.h"
#include "Common/PrimitiveUtils.h"
#include "Material.h"

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

    BEGIN_UNIFORM_BUFFER_STRUCT(FCreateNodeListBlock)
        SHADER_PARAMETER(uint32, MaxLOD)
        SHADER_PARAMETER(uint32, PassLOD)
        SHADER_PARAMETER(float, ScreenSizeDenominator)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FCreatePatchBlock)
        SHADER_PARAMETER(uvec2, LodTextureSize)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FQuadTreeParameters)
        SHADER_PARAMETER(uvec2, NodeCount)
        SHADER_PARAMETER(uint32, LODNum)
        SHADER_PARAMETER(uint32, NumQuadsPerPatch)
        SHADER_PARAMETER(uint32, NumPatchesPerNode)
    END_UNIFORM_BUFFER_STRUCT()

    static unsigned int fromNodeLoctoNodeDescriptionIndex(glm::uvec2 nodeLoc, unsigned int lod, const WorldLodParam &param)
    {
        return param.NodeDescriptionIndexOffset + nodeLoc.x * param.NodeSideNum.y + nodeLoc.y;
    }
    static glm::vec3 GetNodePositionWS(glm::uvec2 nodeLoc, const WorldLodParam &Param)
    {
        // 这里没有实现从显存读取高度图，所以z是0
        glm::vec2 xy_center_coord = (glm::vec2(nodeLoc) + glm::vec2(0.5)) * Param.NodeMeterSize;
        return glm::vec3(xy_center_coord, 0);
    }
    static bool MeetScreenSize(const WorldLodParam &Param, glm::vec3 cameraPos, glm::uvec2 nodeLoc) {
        glm::vec3 positionWS = GetNodePositionWS(nodeLoc, Param);
        float dis = glm::distance(cameraPos, positionWS);
        float nodeSize = std::max(Param.NodeMeterSize.x, Param.NodeMeterSize.y);
        float f = dis / (nodeSize * 1.5);
        if (f < 1)
            return false;
        return true;
    }

    static FRHIVertexInput AccessStreamComponent(const FVertexStreamComponent &Component, uint8 Location)
    {
        FRHIVertexInput VertexInput;
        VertexInput.VertexBuffer = Component.VertexBuffer->VertexBufferRHI.get();
        VertexInput.Location = Location;
        VertexInput.Offset = Component.Offset;
        VertexInput.Stride = Component.Stride;
        VertexInput.Type = Component.Type;
        return VertexInput;
    }

    void FVHMVertexFactory::SetData(const FDataType &InData)
    {
        Data = InData;
    }

    void FVHMVertexFactory::GetVertexInputList(std::vector<FRHIVertexInput> &OutVertexInputs) const
    {
        if (Data.PositionComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.PositionComponent, 0));
        }
        if (Data.ColorComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.ColorComponent, 3));
        }
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
            {
                OutVertexInputs.push_back(AccessStreamComponent(Data.TexCoordComponent[i], 4+i));
            }
        }
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
            , NodeIDs_Final_Maxlength(Component->NumMaxRenderingNodes * sizeof(uvec4))
            , NodeIDs_Temp_Maxlength(Component->NumMaxRenderingNodes * sizeof(uvec2))
        {
            if (Component->Material)
                Material = Component->Material->GetResource();
            if (Component->HeightfieldTexture)
                HeightField = Component->HeightfieldTexture->GetResource()->GetSamplerRHI();
            if (Material == nullptr || HeightField == nullptr)
                return;
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
            }

            PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FVirtualHeightfieldMeshSceneProxy::PreRenderCallBack);
            ENQUEUE_RENDER_COMMAND(FVirtualHeightfieldMeshSceneProxy_Constructor)(
                [this](FDynamicRHI *RHICmdList)
                {
                    NormalTexture = RHICmdList->RHICreateTexture2D(
                        "NormalTexture", 
                        EPixelFormat::PF_R16G16B16A16F, 
                        1, 
                        HeightField->Texture->GetSizeXYZ().x, 
                        HeightField->Texture->GetSizeXYZ().y, 
                        nullptr);
                    NormalSampler.Texture = NormalTexture.get();
                    NormalSampler.Params = HeightField->Params;
                    TangentTexture = RHICmdList->RHICreateTexture2D(
                        "TangentTexture", 
                        EPixelFormat::PF_R16G16B16A16F, 
                        1, 
                        HeightField->Texture->GetSizeXYZ().x, 
                        HeightField->Texture->GetSizeXYZ().y, 
                        nullptr);
                    TangentSampler.Texture = TangentTexture.get();
                    TangentSampler.Params = HeightField->Params;

                    LodTexture = RHICmdList->RHICreateTexture2D(
                        "LodTexture", 
                        EPixelFormat::PF_R16F, 
                        1, 
                        NodeCount.x, 
                        NodeCount.y, 
                        nullptr);
                    uvec2 size = LodParams[0].NodeSideNum * uvec2(8);
                    HeightMinMaxTexture = RHICmdList->RHICreateTexture2D(
                            "HeightMinMaxTexture", 
                            EPixelFormat::PF_R16G16F, 
                            LodCount+3, 
                            size.x, 
                            size.y, 
                            nullptr);
                    for (int i = 0; i < LodCount+3; i++)
                    {
                        HeightMinMaxTextureViews.push_back(RHICmdList->RHICreateTextureView2D(
                            HeightMinMaxTexture.get(),
                            EPixelFormat::PF_R16G16F, 
                            i, 
                            1));
                    }

                    LodParamsBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        LodParams.size() * sizeof(WorldLodParam), 
                        LodParams.data());
                    NodeDescriptionBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NumAllNodes * 4, 
                        0);
                    FinalNodeListBuffer = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Final_Maxlength, nullptr);
                    NodeIDs_TempA = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, nullptr);
                    NodeIDs_TempB = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, nullptr);

                    FinalNodeListIndirectArgs = RHICmdList->RHICreateDispatchIndirectBuffer(0, 1, 1);
                    PatchListBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NodeIDs_Final_Maxlength * 
                        NumPatchesPerNode*NumPatchesPerNode * 
                        sizeof(RenderPatch), nullptr);

                    CreateNodeListBlock = CreateUniformBuffer<FCreateNodeListBlock>();
                    CreateNodeListBlock->Data.MaxLOD = LodCount-1;
                    CreateNodeListBlock->InitRHI();

                    QuadTreeParameters = CreateUniformBuffer<FQuadTreeParameters>();
                    QuadTreeParameters->Data.NumQuadsPerPatch = NumQuadsPerPatch;
                    QuadTreeParameters->Data.NumPatchesPerNode = NumPatchesPerNode;
                    QuadTreeParameters->Data.NodeCount = NodeCount;
                    QuadTreeParameters->Data.LODNum = LodCount;
                    QuadTreeParameters->InitRHI();

                    CreatePatchBlock = CreateUniformBuffer<FCreatePatchBlock>();
                    CreatePatchBlock->Data.LodTextureSize = NodeCount;
                    CreatePatchBlock->InitRHI();

                    DrawIndirectArgs = RHICmdList->RHICreateDrawElementsIndirectBuffer(IndexBuffer.GetNumIndices(), 0, 0, 0, 0);
                    
                    BuildMinMaxTexture();

                    BuildNormalTexture();
                });
        }

        void PreRenderCallBack(FDynamicRHI*, FScene* Scene)
        {
            FViewSceneInfo *ViewInfo = Scene->GetMainCamera();
            CreateNodeListGPU(ViewInfo->SceneProxy->GetViewUniformBuffer());
            CreateLodTexture();
            CreatePatch(ViewInfo->SceneProxy->GetViewUniformBuffer());
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            if (Material == nullptr || HeightField == nullptr)
                return;
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    FMeshBatch Mesh;
                    Mesh.CastShadow = false;
                    Mesh.MaterialRenderProxy = Material->CreateRenderProxy();
                    Mesh.Element.IndexBuffer = &IndexBuffer;
                    Mesh.Element.VertexFactory = &VertexFactory;
                    Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveUniformBuffer.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("Patch_Buffer", PatchListBuffer.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("FQuadTreeParameters", QuadTreeParameters.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("HeightfieldTexture", HeightField);
                    Mesh.Element.Bindings.SetElementShaderBinding("NormalTexture", &NormalSampler);
                    Mesh.Element.Bindings.SetElementShaderBinding("TangentTexture", &TangentSampler);

                    Mesh.Element.NumVertices = 0;
                    Mesh.Element.IndirectArgsBuffer = DrawIndirectArgs.get();
                    Mesh.Element.IndirectArgsOffset = 0;

                    Collector.AddMesh(ViewIndex, Mesh);
                }
            }
        }

        void BuildMinMaxTexture()
        {
            {   // First pass
                FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxFirstPassShader::StaticType, 0);
                FShaderInstance *CreateMinMaxFirstPassShader = GetContentManager()->GetGlobalShader(PermutationParameters);
                FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxFirstPassShader);

                FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                    PSO, EPipelineStage::PS_Compute, 
                    "FQuadTreeParameters", QuadTreeParameters->GetRHI());
                FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                    PSO, EPipelineStage::PS_Compute, 
                    "HeightMap", *HeightField);
                FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                    PSO, EPipelineStage::PS_Compute, 
                    "OutMinMaxMap", HeightMinMaxTextureViews[0].get(), EDataAccessFlag::DA_WriteOnly);
                uvec2 group_num = LodParams[0].NodeSideNum;     // 一个work group负责一个node
                FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);
                FDynamicRHI::GetDynamicRHI()->RHIImageMemoryBarrier();
            }

            FVHMCreateMinMaxShader::FPermutationDomain PermutationVector;
            PermutationVector.Set<FVHMCreateMinMaxShader::FDimensionForPatchMinMax>(true);
            {   // Create MinMax texture for patches
                FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxShader::StaticType, PermutationVector.ToDimensionValueId());
                FShaderInstance *CreateMinMaxShader = GetContentManager()->GetGlobalShader(PermutationParameters);
                FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxShader);
                for (int LOD = 1; LOD < LodCount; LOD++)
                {
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "InMinMaxMap", HeightMinMaxTextureViews[LOD - 1].get(), EDataAccessFlag::DA_ReadOnly);
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "OutMinMaxMap", HeightMinMaxTextureViews[LOD].get(), EDataAccessFlag::DA_WriteOnly);
                    uvec2 group_num = LodParams[LOD].NodeSideNum;     // 一个work group负责一个node
                    FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);
                    FDynamicRHI::GetDynamicRHI()->RHIImageMemoryBarrier();
                }
            }
            PermutationVector.Set<FVHMCreateMinMaxShader::FDimensionForPatchMinMax>(false);
            {   // Create MinMax texture for nodes
                FShaderPermutationParameters PermutationParameters(&FVHMCreateMinMaxShader::StaticType, PermutationVector.ToDimensionValueId());
                FShaderInstance *CreateMinMaxShader = GetContentManager()->GetGlobalShader(PermutationParameters);
                FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateMinMaxShader);
                for (int LOD = LodCount; LOD < LodCount+3; LOD++)
                {
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "InMinMaxMap", HeightMinMaxTextureViews[LOD - 1].get(), EDataAccessFlag::DA_ReadOnly);
                    FDynamicRHI::GetDynamicRHI()->RHISetShaderImage(
                        PSO, EPipelineStage::PS_Compute, 
                        "OutMinMaxMap", HeightMinMaxTextureViews[LOD].get(), EDataAccessFlag::DA_WriteOnly);
                    uvec2 group_num = LodParams[LOD-3].NodeSideNum;     // 一个work group负责一个node
                    FDynamicRHI::GetDynamicRHI()->RHIDispatch(group_num.x, group_num.y, 1);
                    FDynamicRHI::GetDynamicRHI()->RHIImageMemoryBarrier();
                }
            }
        }

        void BuildNormalTexture()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FVHMBuildNormalTangentTextureShader::StaticType, 0);
            FShaderInstance *CreateLodTextureShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(CreateLodTextureShader);

            RHICmdList->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute, 
                "Heightfield", *HeightField);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "NormalTexture", NormalTexture.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "TangentTexture", TangentTexture.get(), EDataAccessFlag::DA_WriteOnly);

            BEGIN_UNIFORM_BUFFER_STRUCT(FBuildNormalTangentBlock)
                SHADER_PARAMETER(uint32, HeightfieldWidth)
                SHADER_PARAMETER(uint32, HeightfieldHeight)
                SHADER_PARAMETER(vec2, PixelMeterSize)
            END_UNIFORM_BUFFER_STRUCT()

            auto BuildNormalTangentBlock = CreateUniformBuffer<FBuildNormalTangentBlock>();
            BuildNormalTangentBlock->Data.HeightfieldWidth = HeightField->Texture->GetSizeXYZ().x;
            BuildNormalTangentBlock->Data.HeightfieldHeight = HeightField->Texture->GetSizeXYZ().y;
            BuildNormalTangentBlock->Data.PixelMeterSize = vec2(NumQuadsPerPatch * NumPatchesPerNode * NodeCount) / vec2(HeightField->Texture->GetSizeXYZ());
            BuildNormalTangentBlock->InitRHI();

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FBuildNormalTangentBlock", BuildNormalTangentBlock->GetRHI());

            RHICmdList->RHIDispatch(HeightField->Texture->GetSizeXYZ().x, HeightField->Texture->GetSizeXYZ().y, 1);
        }

        void CreateNodeListGPU(TUniformBuffer<FViewShaderParameters> *ViewShaderParameters)
        {
            CreateNodeListBlock->Data.ScreenSizeDenominator = 2 * glm::tan(0.5*ViewShaderParameters->Data.CameraVerticalFieldOfView);
            RHIGetError();
            uint32 index_b_value = LodParams[LodCount-1].NodeSideNum.x * LodParams[LodCount-1].NodeSideNum.y;
            RHIBufferRef IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);
            RHIBufferRef IndexFinal = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);

            RHIGetError();
            RHIBufferRef indirectArgs = FDynamicRHI::GetDynamicRHI()->RHICreateDispatchIndirectBuffer(index_b_value, 1, 1);

            RHIGetError();
            FShaderPermutationParameters PermutationParameters(&FVHMCreateNodeListShader::StaticType, 0);
            FShaderInstance *CreateNodeListShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateNodeListShader);
            
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
            RHITextureParams params;
            params.Mag_Filter = ETextureFilters::TF_Nearest;
            params.Min_Filter = ETextureFilters::TF_Nearest;
            FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute, 
                "MinMaxMap", FRHISampler(HeightMinMaxTexture.get(), params));
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

                FDynamicRHI::GetDynamicRHI()->RHIStorageMemoryBarrier();

                RHIGetError();
                FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexB, indirectArgs, 0, 0, 4);
                

                RHIGetError();
                std::swap(NodeIDs_TempA, NodeIDs_TempB);
                IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);
            }

            RHIGetError();
            FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexFinal, FinalNodeListIndirectArgs, 0, 0, 4);
            
        }

        void CreateLodTexture()
        {
            FShaderPermutationParameters PermutationParameters(&FVHMCreateLodTextureShader::StaticType, 0);
            FShaderInstance *CreateLodTextureShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreateLodTextureShader);

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
            AtomicPatchCounterBuffer = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);

            FShaderPermutationParameters PermutationParameters(&FVHMCreatePatchShader::StaticType, 0);
            FShaderInstance *CreatePatchShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(CreatePatchShader);

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
            RHITextureParams params;
            params.Mag_Filter = ETextureFilters::TF_Nearest;
            params.Min_Filter = ETextureFilters::TF_Nearest;
            FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                PSO, EPipelineStage::PS_Compute, 
                "MinMaxMap", FRHISampler(HeightMinMaxTexture.get(), params));
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FCreatePatchBlock", CreatePatchBlock->GetRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
            FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FViewShaderParameters", ViewShaderParameters->GetRHI());

            FDynamicRHI::GetDynamicRHI()->RHIDispatchIndirect(FinalNodeListIndirectArgs.get());
            FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(AtomicPatchCounterBuffer, DrawIndirectArgs, 0, 4, 4);
            
            
        }

	    FMaterial* Material = nullptr;
        FRHISampler* HeightField = nullptr;
        RHITexture2DRef NormalTexture;
        FRHISampler NormalSampler;
        RHITexture2DRef TangentTexture;
        FRHISampler TangentSampler;
        RHITexture2DRef HeightMinMaxTexture;
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

        uint32 AtomicPatchCounter;

        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FVHMVertexFactory VertexFactory;

        TUniformBufferRef<FCreateNodeListBlock> CreateNodeListBlock;
        TUniformBufferRef<FQuadTreeParameters> QuadTreeParameters;
        TUniformBufferRef<FCreatePatchBlock> CreatePatchBlock;

        std::vector<WorldLodParam> LodParams;
		uint32 NodeIDs_Temp_Maxlength = 0, NodeIDs_Final_Maxlength = 0;
		uint32 FinalNodeListSize;
        uint32 NumAllNodes;

        uint32 NumQuadsPerPatch;
        uint32 NumPatchesPerNode;
        uvec2 NodeCount;
        
        uint8 LodCount = 1;
        FDelegateHandle PreRenderHandle;
        FDelegateHandle PostRenderHandle;

    };



    UVirtualHeightfieldMeshComponent::UVirtualHeightfieldMeshComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
    {

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