#include "FourierTransformOcean.h"
#include "RenderingThread.h"
#include "BaseApplication.h"
#include "DynamicMeshResources.h"
#include "PrimitiveUtils.h"
#include "StaticMeshResources.h"
#include "Common/Actor/CameraActor.h"

namespace nilou {

    constexpr int MAX_RENDERING_NODES = 500;

    DECLARE_GLOBAL_SHADER(FOceanGaussionSpectrumShader)
    IMPLEMENT_SHADER_TYPE(FOceanGaussionSpectrumShader, "/Shaders/FastFourierTransformOcean/OceanGaussianSpectrum.comp", EShaderFrequency::SF_Compute, Global);

    DECLARE_GLOBAL_SHADER(FOceanDisplacementSpectrumShader)
    IMPLEMENT_SHADER_TYPE(FOceanDisplacementSpectrumShader, "/Shaders/FastFourierTransformOcean/OceanDisplacementSpectrum.comp", EShaderFrequency::SF_Compute, Global);

	class FOceanFastFourierTransformShader : public FGlobalShader
	{
	public:
		DECLARE_SHADER_TYPE() 
        class FDimensionHorizontalPass : SHADER_PERMUTATION_BOOL("HORIZONTAL_PASS");
        using FPermutationDomain = TShaderPermutationDomain<FDimensionHorizontalPass>;
        static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameter, FShaderCompilerEnvironment& Environment)
        {
            FPermutationDomain Domain(Parameter.PermutationId);
            Domain.ModifyCompilationEnvironment(Environment);
        }
	};
    IMPLEMENT_SHADER_TYPE(FOceanFastFourierTransformShader, "/Shaders/FastFourierTransformOcean/OceanFastFourierTransform.comp", EShaderFrequency::SF_Compute, Global);

    DECLARE_GLOBAL_SHADER(FOceanDisplacementShader)
    IMPLEMENT_SHADER_TYPE(FOceanDisplacementShader, "/Shaders/FastFourierTransformOcean/OceanCreateDisplacement.comp", EShaderFrequency::SF_Compute, Global);

    DECLARE_GLOBAL_SHADER(FOceanNormalFoamShader)
    IMPLEMENT_SHADER_TYPE(FOceanNormalFoamShader, "/Shaders/FastFourierTransformOcean/OceanCreateNormalFoam.comp", EShaderFrequency::SF_Compute, Global);

    class FFourierTransformOceanVertexFactory : public FStaticVertexFactory
    {
        DECLARE_VERTEX_FACTORY_TYPE(FFourierTransformOceanVertexFactory)
    };
    IMPLEMENT_VERTEX_FACTORY_TYPE(FFourierTransformOceanVertexFactory, "/Shaders/VertexFactories/FourierTransformOceanVertexFactory.glsl")

    class FFourierTransformOceanSceneProxy : public FPrimitiveSceneProxy
    {
    
    public:

        FFourierTransformOceanSceneProxy(UFourierTransformOceanComponent* Component)
            : FPrimitiveSceneProxy(Component)
            , NumQuadsPerNode(Component->NumQuadsPerNode)
            , NodeCount(Component->NodeCount)
            , WindDirection(Component->WindDirection)
            , WindSpeed(Component->WindSpeed)
            , FFTPow(Component->FFTPow)
            , Amplitude(Component->Amplitude)
            , DisplacementTextureSize(Component->DisplacementTextureSize)
            , InitialTime(clock())
            , LODParams(Component->LODParams)
        {
            if (Component->Material)
                Material = Component->Material.get();
            else
                Material = GetContentManager()->GetMaterialByPath("/Materials/ColoredMaterial.nasset");
            PerlinNoiseSampler = &GetContentManager()->GetTextureByPath("/Textures/PerlinNoiseTexture.nasset")->GetResource()->SamplerRHI;
            PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FFourierTransformOceanSceneProxy::PreRenderCallback);
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    FMeshBatch Mesh;
                    Mesh.CastShadow = bCastShadow;
                    Mesh.MaterialRenderProxy = Material->GetRenderProxy();
                    Mesh.Element.IndexBuffer = &IndexBuffer;
                    Mesh.Element.VertexFactory = &VertexFactory;
                    Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
                    Mesh.Element.Bindings.SetElementShaderBinding("NodeListBuffer", NodeListBufferRHI.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("LODParamsBuffer", LODParamsBufferRHI.get());
                    Mesh.Element.Bindings.SetElementShaderBinding("DisplaceTexture", &DisplaceSampler);
                    Mesh.Element.Bindings.SetElementShaderBinding("NormalTexture", &NormalSampler);
                    Mesh.Element.Bindings.SetElementShaderBinding("PerlinNoise", PerlinNoiseSampler);

                    Mesh.Element.NumInstances = RenderingNodesThisFrame.size();
                    Mesh.Element.NumVertices = VertexBuffers.Positions.GetNumVertices();

                    Collector.AddMesh(ViewIndex, Mesh);

                }
            }

        }

        void AddRenderingNodeList(const std::vector<uvec4>& RenderingNodes)
        {
            std::lock_guard<std::mutex> lock(MutexNodeListQueue);
            NodeListQueue.push(RenderingNodes);
        }

        void PreRenderCallback(FDynamicRHI* RHICmdList, FScene* Scene)
        {
            std::unique_lock<std::mutex> lock(MutexNodeListQueue);
            if (!NodeListQueue.empty())
            {
                RenderingNodesThisFrame = NodeListQueue.front(); NodeListQueue.pop();
                if (NodeListBufferRHI)
                {
                    RHICmdList->RHIUpdateBuffer(
                        NodeListBufferRHI.get(), 
                        0, RenderingNodesThisFrame.size()*sizeof(uvec4), 
                        RenderingNodesThisFrame.data());
                }
            }
            lock.unlock();
            auto CurrentTime = clock();
            FFTParameters->Data.Time = float(CurrentTime - InitialTime) / 1000.f + 1000.f;
            FFTParameters->UpdateUniformBuffer();
            RHIGetError();
            CreateDisplacementSpectrum();
            for (int m = 1; m <= FFTPow; m++)
            {
                unsigned int Ns = pow(2, m - 1);
                RHIGetError();
                FastFourierTransform(Ns, HeightSpectrumRT, true);
                FastFourierTransform(Ns, DisplaceXSpectrumRT, true);
                FastFourierTransform(Ns, DisplaceYSpectrumRT, true);
            }
            for (int m = 1; m <= FFTPow; m++)
            {
                unsigned int Ns = pow(2, m - 1);
                RHIGetError();
                FastFourierTransform(Ns, HeightSpectrumRT, false);
                FastFourierTransform(Ns, DisplaceXSpectrumRT, false);
                FastFourierTransform(Ns, DisplaceYSpectrumRT, false);
            }
            RHIGetError();
            CreateDisplacement();
            RHIGetError();
            CreateNormalFoam();
        }

        void CreateDisplacementSpectrum()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanDisplacementSpectrumShader::StaticType, 0);
            FShaderInstance *DisplacementSpectrumShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(DisplacementSpectrumShader->GetComputeShaderRHI());

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFastFourierTransformParameters", FFTParameters->GetRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "GaussianRandomRT", GaussianRandomRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "HeightSpectrumRT", HeightSpectrumRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "DisplaceXSpectrumRT", DisplaceXSpectrumRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "DisplaceYSpectrumRT", DisplaceYSpectrumRT.get(), EDataAccessFlag::DA_WriteOnly);
            
            RHICmdList->RHIDispatch(group_num, group_num, 1);

        }

        void FastFourierTransform(uint32 Ns, RHITexture2DRef& InputRT, bool bHorizontalPass)
        {
            FOceanFastFourierTransformShader::FPermutationDomain PermutationVector;
            if (bHorizontalPass)
                PermutationVector.Set<FOceanFastFourierTransformShader::FDimensionHorizontalPass>(true);
            else
                PermutationVector.Set<FOceanFastFourierTransformShader::FDimensionHorizontalPass>(false);
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanFastFourierTransformShader::StaticType, PermutationVector.ToDimensionValueId());
            FShaderInstance *FFTShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(FFTShader->GetComputeShaderRHI());

            ButterflyBlock->Data.Ns = Ns;
            ButterflyBlock->UpdateUniformBuffer();

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFastFourierTransformParameters", FFTParameters->GetRHI());

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFFTButterflyBlock", ButterflyBlock->GetRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "InputRT", InputRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "OutputRT", IntermediateRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);

            std::swap(InputRT, IntermediateRT);

        }

        void CreateDisplacement()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanDisplacementShader::StaticType, 0);
            FShaderInstance *DisplacementShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(DisplacementShader->GetComputeShaderRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "HeightSpectrumRT", HeightSpectrumRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "DisplaceXSpectrumRT", DisplaceXSpectrumRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "DisplaceYSpectrumRT", DisplaceYSpectrumRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "DisplaceRT", DisplaceRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);
            RHICmdList->RHIGenerateMipmap(DisplaceRT);

        }

        void CreateNormalFoam()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanNormalFoamShader::StaticType, 0);
            FShaderInstance *NormalFoamShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(NormalFoamShader->GetComputeShaderRHI());

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFastFourierTransformParameters", FFTParameters->GetRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "DisplaceRT", DisplaceRT.get(), EDataAccessFlag::DA_ReadOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "NormalRT", NormalRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute, 
                "FoamRT", FoamRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);
            RHICmdList->RHIGenerateMipmap(NormalRT);
        }

        virtual void CreateRenderThreadResources() override
        {
            FPrimitiveSceneProxy::CreateRenderThreadResources();
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildFlatSurfaceVerts(uvec2(NumQuadsPerNode+3), OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);
            BeginInitResource(&IndexBuffer);
            VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);
            LODParamsBufferRHI = RHICmdList->RHICreateShaderStorageBuffer(
                LODParams.size() * sizeof(UFourierTransformOceanComponent::OceanLODParam),
                LODParams.data());
            NodeListBufferRHI = RHICmdList->RHICreateShaderStorageBuffer(
                MAX_RENDERING_NODES * sizeof(uvec4), nullptr);


            N = glm::pow(2, FFTPow);
            group_num = N / 32;     // 这里的32是写死在glsl中的"local_size"
            GaussianRandomRT = RHICmdList->RHICreateTexture2D("GaussianRandomRT", EPixelFormat::PF_R16G16F, 1, N, N, TexCreate_UAV);
            HeightSpectrumRT = RHICmdList->RHICreateTexture2D("HeightSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, TexCreate_UAV);
            DisplaceXSpectrumRT = RHICmdList->RHICreateTexture2D("DisplaceXSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, TexCreate_UAV);
            DisplaceYSpectrumRT = RHICmdList->RHICreateTexture2D("DisplaceYSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, TexCreate_UAV);
            IntermediateRT = RHICmdList->RHICreateTexture2D("IntermediateRT", EPixelFormat::PF_R16G16F, 1, N, N, TexCreate_UAV);
            DisplaceRT = RHICmdList->RHICreateTexture2D("DisplaceRT", EPixelFormat::PF_R16G16B16A16F, FFTPow, N, N, TexCreate_UAV);
            NormalRT = RHICmdList->RHICreateTexture2D("NormalRT", EPixelFormat::PF_R16G16B16A16F, FFTPow, N, N, TexCreate_UAV);
            FoamRT = RHICmdList->RHICreateTexture2D("FoamRT", EPixelFormat::PF_R16F, FFTPow, N, N, TexCreate_UAV);

            DisplaceSampler = FRHISampler(DisplaceRT.get());
            NormalSampler = FRHISampler(NormalRT.get());
            
            FFTParameters = CreateUniformBuffer<FOceanFastFourierTransformParameters>();
            FFTParameters->Data.DisplacementTextureSize = DisplacementTextureSize;
            FFTParameters->Data.Amplitude = Amplitude;
            FFTParameters->Data.N = N;
            FFTParameters->Data.WindDirection = WindDirection;
            FFTParameters->Data.WindSpeed = WindSpeed;
            FFTParameters->InitResource();

            ButterflyBlock = CreateUniformBuffer<FOceanFFTButterflyBlock>();
            ButterflyBlock->InitResource();
            
            FShaderPermutationParameters PermutationParameters(&FOceanGaussionSpectrumShader::StaticType, 0);
            FShaderInstance *GaussionSpectrumShader = GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(GaussionSpectrumShader->GetComputeShaderRHI());

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFastFourierTransformParameters", FFTParameters->GetRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "GaussianRandomRT", GaussianRandomRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);

            Material->SetParameterValue("FOceanFastFourierTransformParameters", FFTParameters.get());


        }

        virtual void DestroyRenderThreadResources() override
        {
            GetAppication()->GetPreRenderDelegate().Remove(PreRenderHandle);
            VertexBuffers.ReleaseResource();
            IndexBuffer.ReleaseResource();
            FFTParameters->ReleaseResource();
            ButterflyBlock->ReleaseResource();
            FPrimitiveSceneProxy::DestroyRenderThreadResources();
        }

	    UMaterial* Material = nullptr;

        uint32 NumQuadsPerNode;

        uint32 NodeCount;

		vec2 WindDirection;

        float WindSpeed;

        uint32 FFTPow;

        float Amplitude;

        float DisplacementTextureSize;

        uint32 group_num;

        uint32 N;    // N = pow(2, FFTPow)

        clock_t InitialTime;

        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FFourierTransformOceanVertexFactory VertexFactory;

		RHITexture2DRef GaussianRandomRT;          // 高斯随机数
		RHITexture2DRef HeightSpectrumRT;          // 高度频谱
		RHITexture2DRef DisplaceXSpectrumRT;       // X偏移频谱
		RHITexture2DRef DisplaceYSpectrumRT;       // Y偏移频谱
		RHITexture2DRef DisplaceRT;                // 偏移纹理
		RHITexture2DRef IntermediateRT;            // 临时储存输出纹理
		RHITexture2DRef NormalRT;                  // 法线
		RHITexture2DRef FoamRT;					   // 白沫

        FRHISampler DisplaceSampler;
        FRHISampler NormalSampler;
        FRHISampler FoamSampler;

		FRHISampler* PerlinNoiseSampler;

        TUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters;
        TUniformBufferRef<FOceanFFTButterflyBlock> ButterflyBlock;

        std::vector<uvec4> RenderingNodesThisFrame;

        RHIBufferRef NodeListBufferRHI;
        RHIBufferRef LODParamsBufferRHI;

        std::vector<UFourierTransformOceanComponent::OceanLODParam> LODParams;

    private:

        FDelegateHandle PreRenderHandle;

        std::queue<std::vector<uvec4>> NodeListQueue;
        std::mutex MutexNodeListQueue;

    };

    UFourierTransformOceanComponent::UFourierTransformOceanComponent()
        : Material(GetContentManager()->GetMaterialByPath("/Materials/OceanMaterial.nasset")->CreateMaterialInstance())
    {
        uint32 temp_NodeCount = NodeCount;
        while (temp_NodeCount % 2 == 0)
        {
            LodCount++;
            temp_NodeCount /= 2;
        }
        LODParams.resize(LodCount);
        vec2 Lod0NodeMeterSize = vec2(NumQuadsPerNode);
        vec2 LandscapeMeterSize = Lod0NodeMeterSize * vec2(NodeCount); 
        for (int lod = 0; lod < LodCount; lod++)
        {
            OceanLODParam param;
            param.NodeMeterSize = Lod0NodeMeterSize * vec2(glm::pow(2, lod));
            param.NodeSideNum = NodeCount / uvec2(glm::pow(2, lod));
            LODParams[lod] = param;
        }
    }

    void UFourierTransformOceanComponent::TickComponent(double DeltaTime)
    {
        UWorld* World = GetWorld();
        if (World && World->GetFirstCameraActor())
        {
            UCameraComponent* CameraComponent = World->GetFirstCameraActor()->GetCameraComponent();
            FViewFrustum Frustum = FViewFrustum(
                CameraComponent->GetComponentLocation(), 
                CameraComponent->GetForwardVector(), 
                CameraComponent->GetUpVector(), 
                CameraComponent->GetAspectRatio(), 
                CameraComponent->VerticalFieldOfView, 
                CameraComponent->NearClipDistance, 
                CameraComponent->FarClipDistance);
            std::vector<uvec4> NodeListFinal = CreateNodeList(Frustum, CameraComponent->GetComponentLocation());
            FFourierTransformOceanSceneProxy* Proxy = (FFourierTransformOceanSceneProxy*)SceneProxy;
            if (Proxy)
            {
                Proxy->AddRenderingNodeList(NodeListFinal);
            }
        }
    }

    std::vector<uvec4> UFourierTransformOceanComponent::CreateNodeList(const FViewFrustum &Frustum, const dvec3& CameraPosition)
    {
        std::vector<uvec2> NodeIDs_TempA;
        std::vector<uvec2> NodeIDs_TempB;
        std::vector<uvec4> NodeListFinal;
        std::mutex MutexNodeListB;
        std::mutex MutexNodeListFinal;
        for (int i = 0; i < LODParams[LodCount-1].NodeSideNum.x; i++)
        {
            for (int j = 0; j < LODParams[LodCount-1].NodeSideNum.y; j++)
            {
                NodeIDs_TempA.push_back(uvec2(i, j));
            }
        }
        for (int lod = LodCount - 1; lod >= 0; lod--)
        {
            for (int i = 0; i < NodeIDs_TempA.size(); i++)
            {
                uvec2 nodeLoc = NodeIDs_TempA[i];
                pool.push_task(
                [&MutexNodeListB, &MutexNodeListFinal, &NodeListFinal, &Frustum, nodeLoc, lod, CameraPosition, this, &NodeIDs_TempB](){

                    vec2 node_size = vec2(LODParams[lod].NodeMeterSize.x, LODParams[lod].NodeMeterSize.y);
                    vec2 node_offset = vec2(nodeLoc) * node_size;
                    vec2 MinMax_uv = (vec2(nodeLoc) + vec2(0.5)) / vec2(LODParams[lod].NodeSideNum.x, LODParams[lod].NodeSideNum.y);
                    float estimated_wave_height = Amplitude / (0.45f * 1e-3f);
                    dmat3 HalfAxes = dmat3(
                        GetComponentToWorld().TransformVector(dvec3(node_size.x*0.5, 0, 0)), 
                        GetComponentToWorld().TransformVector(dvec3(0, node_size.y*0.5, 0)), 
                        GetComponentToWorld().TransformVector(dvec3(0, 0, estimated_wave_height*0.5)));
                    dvec3 BoxCenter = GetComponentToWorld().TransformPosition(dvec3(node_offset+node_size/2.f, estimated_wave_height*0.5));
                    bool bFrustumCulled = Frustum.IsBoxOutSideFrustumFast(BoxCenter, HalfAxes);
                    if (bFrustumCulled)
                        return;

                    if (lod > 0 && !MeetScreenSize(CameraPosition, nodeLoc, LODParams[lod])) 
                    {
                        std::lock_guard<std::mutex> lock(MutexNodeListB);
                        NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2);
                        NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + uvec2(1, 0));
                        NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + uvec2(0, 1));
                        NodeIDs_TempB.push_back(nodeLoc * (unsigned int)2 + uvec2(1, 1));
                    }
                    else 
                    {
                        std::lock_guard<std::mutex> lock(MutexNodeListFinal);
                        NodeListFinal.push_back(uvec4(nodeLoc, lod, 0));
                    }
                });
            }
            pool.wait_for_tasks();
            std::swap(NodeIDs_TempA, NodeIDs_TempB);
            NodeIDs_TempB.clear();
        }
        return NodeListFinal;

    }

    FBoundingBox UFourierTransformOceanComponent::CalcBounds(const FTransform& LocalToWorld) const
    {
        // Ocean surface size (in world unit)
        float SurfaceSize = NumQuadsPerNode * NodeCount;
        float estimated_wave_height = Amplitude / (0.45f * 1e-3f);
        return FBoundingBox(vec3(0), vec3(SurfaceSize, SurfaceSize, estimated_wave_height)).TransformBy(LocalToWorld);
    }

    FPrimitiveSceneProxy* UFourierTransformOceanComponent::CreateSceneProxy()
    {
        return new FFourierTransformOceanSceneProxy(this);
    }

}