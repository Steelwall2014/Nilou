#include "VirtualHeightfieldMeshComponent.h"
#include "Common/BaseApplication.h"

namespace nilou {
    
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
        SHADER_PARAMETER(vec3, CameraPosition)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FCreatePatchBlock)
        SHADER_PARAMETER_ARRAY(vec4, 6, FrustumPlanes)
        SHADER_PARAMETER(uint32, LodTextureSize)
    END_UNIFORM_BUFFER_STRUCT()

    static unsigned int fromNodeLoctoNodeDescriptionIndex(glm::uvec2 nodeLoc, unsigned int lod, const WorldLodParam &param)
    {
        return param.NodeDescriptionIndexOffset + nodeLoc.y * param.NodeSideNum.x + nodeLoc.x;
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

    class FVirtualHeightfieldMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FVirtualHeightfieldMeshSceneProxy(UVirtualHeightfieldMeshComponent *Component)
            : FPrimitiveSceneProxy(Component)
        {
            uvec2 NodeCount = Component->NodeCount;
            while (NodeCount.x % 2 == 0 && NodeCount.y % 2 == 0)
            {
                LodCount++;
                NodeCount.x /= 2;
                NodeCount.y /= 2;
            }
            LodParams.resize(LodCount);
            vec2 Lod0NodeMeterSize = vec2(Component->GetComponentScale()) * vec2(Component->NumSectionsPerNode) * vec2(Component->NumQuadsPerSection);
            vec2 LandscapeMeterSize = Lod0NodeMeterSize * vec2(Component->NodeCount); 

            int NodeNum = 0;
            for (int lod = 0; lod < LodCount; lod++)
            {
                WorldLodParam param;
                param.NodeMeterSize = Lod0NodeMeterSize * vec2(glm::pow(2, lod));
                param.NodeSideNum = Component->NodeCount / uvec2(glm::pow(2, lod));
                LodParams[lod] = param;
                NodeNum += param.NodeSideNum.x * param.NodeSideNum.y;
            }
            if (bUseCpuDivision)
                NodeDescription.resize(NodeNum);

            LodParams[0].NodeDescriptionIndexOffset = 0;
            for (int lod = 1; lod < LodCount; lod++)
            {
                LodParams[lod].NodeDescriptionIndexOffset = 
                    LodParams[lod-1].NodeDescriptionIndexOffset + 
                    LodParams[lod-1].NodeSideNum.x * LodParams[lod-1].NodeSideNum.y;
            }

            // 由于opengl并不支持在运行时动态分配内存(至少我没有找到)，也就实现不了push_back之类的功能
            // 因此需要预先分配空间，再使用一个atomic counter来间接实现这种功能，就需要预先计算最大长度。
            // 下面是为了计算NodeList TempA、NodeList TempB和NodeList Final最大的长度
            // 显然当摄像机在平面正中央，而且正好在平面上时，NodeList会是最长的
            glm::vec3 cameraPos(LandscapeMeterSize.x / 2, LandscapeMeterSize.y / 2, 0);
            CreateNodeListCPU(cameraPos); 

            ENQUEUE_RENDER_COMMAND(FVirtualHeightfieldMeshSceneProxy_Constructor)(
                [this, Component, NodeNum](FDynamicRHI *RHICmdList)
                {
                    LodTexture = RHICmdList->RHICreateTexture2D(
                        "LodTexture", 
                        EPixelFormat::PF_R16F, 
                        1, 
                        Component->NodeCount.x, 
                        Component->NodeCount.y, 
                        nullptr);
                    for (int i = 0; i < LodCount+3; i++)
                    {
                        uvec2 size;
                        if (i < LodCount)
                            size = LodParams[i].NodeSideNum * uvec2(8);
                        else
                            size = LodParams[i-3].NodeSideNum;
                        HeightMinMaxTextures.push_back(RHICmdList->RHICreateTexture2D(
                            "HeightMinMaxTexture_"+std::to_string(i), 
                            EPixelFormat::PF_R16F, 
                            1, 
                            size.x, 
                            size.y, 
                            nullptr));
                    }

                    LodParamsBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        LodParams.size() * sizeof(WorldLodParam), 
                        LodParams.data());
                    NodeDescriptionBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NodeNum * 4, 
                        0);
                    FinalNodeListBuffer = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Final_Maxlength, nullptr);
                    NodeIDs_TempA = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, nullptr);
                    NodeIDs_TempB = RHICmdList->RHICreateShaderStorageBuffer(NodeIDs_Temp_Maxlength, nullptr);

                    FinalNodeListIndirectArgs = RHICmdList->RHICreateDispatchIndirectBuffer(0, 1, 1);
                    PatchListBuffer = RHICmdList->RHICreateShaderStorageBuffer(
                        NodeIDs_Final_Maxlength * 
                        Component->NumSectionsPerNode*Component->NumSectionsPerNode * 
                        sizeof(RenderPatch), nullptr);

                    CreateNodeListBlock = CreateUniformBuffer<FCreateNodeListBlock>();
                    CreateNodeListBlock->InitRHI();
                });


            // PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FVirtualHeightfieldMeshSceneProxy::PreRenderCallBack);
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    CreateNodeListGPU(Views[ViewIndex].Position);
                }
            }
        }

        void CreateNodeListCPU(const vec3 &cameraPos)
        {
            std::vector<glm::uvec2> NodeIDs_TempA;
            std::vector<glm::uvec2> NodeIDs_TempB;
            NodeList_Final.clear();
            for (int i = 0; i < LodParams[LodCount-1].NodeSideNum.x; i++)
            {
                for (int j = 0; j < LodParams[LodCount-1].NodeSideNum.y; j++)
                {
                    NodeIDs_TempA.push_back(glm::uvec2(i, j));
                }
            }
            NodeIDs_Temp_Maxlength = std::max(NodeIDs_Temp_Maxlength, (unsigned int)NodeIDs_TempA.size());
            for (int lod = LodCount - 1; lod >= 0; lod--)
            {
                while (!NodeIDs_TempA.empty())
                {
                    glm::uvec2 nodeLoc = NodeIDs_TempA.back(); NodeIDs_TempA.pop_back();
                    unsigned int index = fromNodeLoctoNodeDescriptionIndex(nodeLoc, lod, LodParams[lod]);
                    if (lod > 0 && !MeetScreenSize(LodParams[lod], cameraPos, nodeLoc)) 
                    {
                        //divide
                        NodeIDs_TempB.push_back(nodeLoc * (uint32)2);
                        NodeIDs_TempB.push_back(nodeLoc * (uint32)2 + glm::uvec2(1, 0));
                        NodeIDs_TempB.push_back(nodeLoc * (uint32)2 + glm::uvec2(0, 1));
                        NodeIDs_TempB.push_back(nodeLoc * (uint32)2 + glm::uvec2(1, 1));

                        if (bUseCpuDivision)
                            NodeDescription[index] = true;

                    }
                    else 
                    {
                        NodeList_Final.push_back(glm::uvec3(nodeLoc, lod));

                        if (bUseCpuDivision)
                            NodeDescription[index] = false;
                    }
                }
                NodeIDs_Temp_Maxlength = std::max(NodeIDs_Temp_Maxlength, (unsigned int)NodeIDs_TempB.size());
                std::swap(NodeIDs_TempA, NodeIDs_TempB);
            }
            if (bUseCpuDivision)
                FinalNodeListSize = NodeIDs_Final_Maxlength = NodeList_Final.size();

            // 留一点冗余
            NodeIDs_Temp_Maxlength *= sizeof(glm::uvec2) * 1.5;
            NodeIDs_Final_Maxlength *= sizeof(glm::uvec3) * 1.5;
        }

        void CreateNodeListGPU(const vec3 &CameraPosition)
        {
            uint32 index_b_value = LodParams[LodCount-1].NodeSideNum.x * LodParams[LodCount-1].NodeSideNum.y;
            RHIBufferRef IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);
            RHIBufferRef IndexFinal = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);

            RHIBufferRef indirectArgs = FDynamicRHI::GetDynamicRHI()->RHICreateDispatchIndirectBuffer(index_b_value, 1, 1);

            FShaderPermutationParameters PermutationParameters(&FVHMCreateNodeListShader::StaticType, 0);
            FShaderInstance *TransmittanceShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = FDynamicRHI::GetDynamicRHI()->RHISetComputeShader(TransmittanceShader);
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(2, FinalNodeListBuffer);
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(4, IndexFinal);
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(5, LodParamsBuffer);
            FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(6, NodeDescriptionBuffer);
            CreateNodeListBlock->Data.MaxLOD = LodCount-1;
            CreateNodeListBlock->Data.CameraPosition = CameraPosition;
            // for (int i = 0; i < 6; i++)
                // CreateNodeListBlock->Data.FrustumPlanes[i] = vec4(Frustum.Planes[i].Normal, Frustum.Planes[i].Distance);
            
            RHITextureParams params;
            params.Mag_Filter = ETextureFilters::TF_Nearest;
            params.Min_Filter = ETextureFilters::TF_Nearest;
            for (int LOD = 0; LOD < HeightMinMaxTextures.size(); LOD++)
            {
                std::string uniformName = std::format("MinMaxMap[{}]", LOD);
                FDynamicRHI::GetDynamicRHI()->RHISetShaderSampler(
                    PSO, EPipelineStage::PS_Compute, 
                    uniformName, FRHISampler(HeightMinMaxTextures[LOD].get(), params));
            }

            for (int lod = LodCount - 1; lod >= 0; lod--)
            {
                CreateNodeListBlock->Data.PassLOD = lod;
                CreateNodeListBlock->UpdateUniformBuffer();
                FDynamicRHI::GetDynamicRHI()->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "FCreateNodeListBlock", CreateNodeListBlock->GetRHI());
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(0, NodeIDs_TempA);
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(1, NodeIDs_TempB);
                FDynamicRHI::GetDynamicRHI()->RHIBindComputeBuffer(3, IndexB);

                if (bUseCpuDivision)
                    FDynamicRHI::GetDynamicRHI()->RHIDispatch(index_b_value, 1, 1);
                else
                    FDynamicRHI::GetDynamicRHI()->RHIDispatchIndirect(indirectArgs.get());

                FDynamicRHI::GetDynamicRHI()->RHIStorageMemoryBarrier();

                if (bUseCpuDivision)
                {
                    unsigned int *ptr = (unsigned int *)FDynamicRHI::GetDynamicRHI()->RHIMapComputeBuffer(IndexB, EDataAccessFlag::DA_ReadOnly);
                    index_b_value = *ptr;
                    FDynamicRHI::GetDynamicRHI()->RHIUnmapComputeBuffer(IndexB);
                }
                else
                {
                    FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexB, indirectArgs, 0, 0, 4);
                }

                std::swap(NodeIDs_TempA, NodeIDs_TempB);
                IndexB = FDynamicRHI::GetDynamicRHI()->RHICreateAtomicCounterBuffer(0);
            }

            if (bUseCpuDivision)
            {
                unsigned int *ptr = (unsigned int *)FDynamicRHI::GetDynamicRHI()->RHIMapComputeBuffer(IndexFinal, EDataAccessFlag::DA_ReadOnly);
                FinalNodeListSize = *ptr;
                FDynamicRHI::GetDynamicRHI()->RHIUnmapComputeBuffer(IndexFinal);
            }
            else
            {
                FDynamicRHI::GetDynamicRHI()->RHICopyBufferSubData(IndexFinal, FinalNodeListIndirectArgs, 0, 0, 4);
            }
        }

        void PreRenderCallBack(FDynamicRHI*)
        {

        }

	    FMaterialRenderProxy* Material;
        RHITexture2DRef HeightField;
        std::vector<RHITexture2DRef> HeightMinMaxTextures;
	    RHITexture2DRef LodTexture;

		RHIBufferRef LodParamsBuffer;
		RHIBufferRef NodeDescriptionBuffer;
		RHIBufferRef NodeIDs_TempA;
		RHIBufferRef NodeIDs_TempB;
		RHIBufferRef FinalNodeListBuffer;
		RHIBufferRef FinalNodeListIndirectArgs;
		RHIBufferRef PatchListBuffer;
		RHIBufferRef AtomicPatchCounterBuffer;

        TUniformBufferRef<FCreateNodeListBlock> CreateNodeListBlock;

        std::vector<WorldLodParam> LodParams;
		std::vector<glm::uvec3> NodeList_Final;
        bool bUseCpuDivision;
		std::vector<uint32> NodeDescription;
		unsigned int NodeIDs_Temp_Maxlength = 0, NodeIDs_Final_Maxlength = 0;
		unsigned int FinalNodeListSize;
        
    private:
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
        return FBoundingBox(vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f)).TransformBy(LocalToWorld);
    }

    FPrimitiveSceneProxy *UVirtualHeightfieldMeshComponent::CreateSceneProxy()
    {
        return new FVirtualHeightfieldMeshSceneProxy(this);
    }


}