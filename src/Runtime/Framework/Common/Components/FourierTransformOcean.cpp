#include "FourierTransformOcean.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "GenerateMips.h"
#include "Texture2D.h"

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

    static void CreateGaussianRandom(RenderGraph& Graph, TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters, RDGTexture* OutGaussianRandomRT)
    {
        int32 group_num = OutGaussianRandomRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanGaussionSpectrumShader::StaticType, 0);
        FShaderInstance *GaussionSpectrumShader = GetGlobalShader(PermutationParameters);
        FRHIPipelineState* PSO = RHICreateComputePipelineState(GaussionSpectrumShader->GetComputeShaderRHI());

        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FOceanGaussionSpectrumShader>(0, 0);
        DescriptorSet->SetStorageBuffer("FOceanFastFourierTransformParameters", FFTParameters.get());
        DescriptorSet->SetStorageImage("GaussianRandomRT", OutGaussianRandomRT->GetDefaultView());

        RDGComputePassDesc PassDesc;
        PassDesc.Name = "CreateGaussionSpectrum";
        PassDesc.DescriptorSets = { DescriptorSet };
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );
    }

    static std::tuple<RDGTexture*, RDGTexture*, RDGTexture*> CreateDisplacementSpectrum(RenderGraph& Graph, RDGTexture* GaussianRandomRT, TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters)
    {
        int32 group_num = GaussianRandomRT->Desc.SizeX / 32;
        RDGTexture* HeightSpectrumRT = Graph.CreateTexture("FastFourierTransform HeightSpectrumRT", GaussianRandomRT->Desc);
        RDGTexture* DisplaceXSpectrumRT = Graph.CreateTexture("FastFourierTransform DisplaceXSpectrumRT", GaussianRandomRT->Desc);
        RDGTexture* DisplaceYSpectrumRT = Graph.CreateTexture("FastFourierTransform DisplaceYSpectrumRT", GaussianRandomRT->Desc);

        FShaderPermutationParameters PermutationParameters(&FOceanDisplacementSpectrumShader::StaticType, 0);
        FShaderInstance *DisplacementSpectrumShader = GetGlobalShader(PermutationParameters);
        FRHIPipelineState* PSO = RHICreateComputePipelineState(DisplacementSpectrumShader->GetComputeShaderRHI());
        
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FOceanDisplacementSpectrumShader>(0, 0);
        DescriptorSet->SetUniformBuffer("FOceanFastFourierTransformParameters", FFTParameters.get());
        DescriptorSet->SetStorageImage("GaussianRandomRT", GaussianRandomRT->GetDefaultView());
        DescriptorSet->SetStorageImage("HeightSpectrumRT", HeightSpectrumRT->GetDefaultView());
        DescriptorSet->SetStorageImage("DisplaceXSpectrumRT", DisplaceXSpectrumRT->GetDefaultView());
        DescriptorSet->SetStorageImage("DisplaceYSpectrumRT", DisplaceYSpectrumRT->GetDefaultView());

        RDGComputePassDesc PassDesc;
        PassDesc.Name = "CreateDisplacementSpectrum";
        PassDesc.DescriptorSets = { DescriptorSet };
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        return { HeightSpectrumRT, DisplaceXSpectrumRT, DisplaceYSpectrumRT };
    }

    static RDGTexture* FastFourierTransform(RenderGraph& Graph, uint32 Ns, RDGTexture* InputRT, TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters, bool bHorizontalPass)
    {
        int32 group_num = InputRT->Desc.SizeX / 32;
        FOceanFastFourierTransformShader::FPermutationDomain PermutationVector;
        if (bHorizontalPass)
            PermutationVector.Set<FOceanFastFourierTransformShader::FDimensionHorizontalPass>(true);
        else
            PermutationVector.Set<FOceanFastFourierTransformShader::FDimensionHorizontalPass>(false);
        FShaderPermutationParameters PermutationParameters(&FOceanFastFourierTransformShader::StaticType, PermutationVector.ToDimensionValueId());
        FShaderInstance *FFTShader = GetGlobalShader(PermutationParameters);
        FRHIPipelineState* PSO = RHICreateComputePipelineState(FFTShader->GetComputeShaderRHI());

        TRDGUniformBuffer<FOceanFFTButterflyBlock>* ButterflyBlock = Graph.CreateUniformBuffer<FOceanFFTButterflyBlock>("FOceanFFTButterflyBlock");
        ButterflyBlock->SetData<&FOceanFFTButterflyBlock::Ns>(Ns);

        RDGTexture* OutputRT = Graph.CreateTexture("FastFourierTransform OutputRT", InputRT->Desc);
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FOceanFastFourierTransformShader>(PermutationVector.ToDimensionValueId(), 0);
        DescriptorSet->SetUniformBuffer("FOceanFastFourierTransformParameters", FFTParameters.get());
        DescriptorSet->SetStorageBuffer("FOceanFFTButterflyBlock", ButterflyBlock);
        DescriptorSet->SetStorageImage("InputRT", InputRT->GetDefaultView());
        DescriptorSet->SetStorageImage("OutputRT", OutputRT->GetDefaultView());

        RDGComputePassDesc PassDesc;
        PassDesc.Name = "FastFourierTransform";
        PassDesc.DescriptorSets = { DescriptorSet };
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        return OutputRT;
    }

    static void CreateDisplacement(RenderGraph& Graph, RDGTexture* HeightSpectrumRT, RDGTexture* DisplaceXSpectrumRT, RDGTexture* DisplaceYSpectrumRT, RDGTexture* OutDisplaceRT)
    {
        int32 group_num = HeightSpectrumRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanDisplacementShader::StaticType, 0);
        FShaderInstance *DisplacementShader = GetGlobalShader(PermutationParameters);
        FRHIPipelineState* PSO = RHICreateComputePipelineState(DisplacementShader->GetComputeShaderRHI());

        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FOceanDisplacementShader>(0, 0);
        DescriptorSet->SetStorageImage("HeightSpectrumRT", HeightSpectrumRT->GetDefaultView());
        DescriptorSet->SetStorageImage("DisplaceXSpectrumRT", DisplaceXSpectrumRT->GetDefaultView());
        DescriptorSet->SetStorageImage("DisplaceYSpectrumRT", DisplaceYSpectrumRT->GetDefaultView());
        DescriptorSet->SetStorageImage("DisplaceRT", OutDisplaceRT->GetDefaultView());

        RDGComputePassDesc PassDesc;
        PassDesc.Name = "CreateDisplacement";
        PassDesc.DescriptorSets = { DescriptorSet };
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        FGenerateMips::Execute(Graph, OutDisplaceRT);
    }

    static void CreateNormalFoam(RenderGraph& Graph, RDGTexture* DisplaceRT, TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters, RDGTexture* OutNormalRT, RDGTexture* OutFoamRT)
    {
        int32 group_num = DisplaceRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanNormalFoamShader::StaticType, 0);
        FShaderInstance *NormalFoamShader = GetGlobalShader(PermutationParameters);
        FRHIPipelineState* PSO = RHICreateComputePipelineState(NormalFoamShader->GetComputeShaderRHI());

        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FOceanNormalFoamShader>(0, 0);
        DescriptorSet->SetUniformBuffer("FOceanFastFourierTransformParameters", FFTParameters.get());
        DescriptorSet->SetStorageImage("DisplaceRT", DisplaceRT->GetDefaultView());
        DescriptorSet->SetStorageImage("NormalRT", OutNormalRT->GetDefaultView());
        DescriptorSet->SetStorageImage("FoamRT", OutFoamRT->GetDefaultView());

        RDGComputePassDesc PassDesc;
        PassDesc.Name = "CreateNormalFoam";
        PassDesc.DescriptorSets = { DescriptorSet };
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        FGenerateMips::Execute(Graph, OutNormalRT);
    }

    static void UpdateHeightField_RenderThread(
        RenderGraph& Graph, int32 FFTPow, RDGTexture* GaussianRandomRT, TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters,
        RDGTexture* OutDisplaceRT, RDGTexture* OutNormalRT, RDGTexture* OutFoamRT)
    {
        auto [HeightSpectrumRT, DisplaceXSpectrumRT, DisplaceYSpectrumRT] = CreateDisplacementSpectrum(Graph, GaussianRandomRT, FFTParameters);
        
        for (int m = 1; m <= FFTPow; m++)
        {
            unsigned int Ns = pow(2, m - 1);
            HeightSpectrumRT = FastFourierTransform(Graph, Ns, HeightSpectrumRT, FFTParameters, true);
            DisplaceXSpectrumRT = FastFourierTransform(Graph, Ns, DisplaceXSpectrumRT, FFTParameters, true);
            DisplaceYSpectrumRT = FastFourierTransform(Graph, Ns, DisplaceYSpectrumRT, FFTParameters, true);
        }
        for (int m = 1; m <= FFTPow; m++)
        {
            unsigned int Ns = pow(2, m - 1);
            HeightSpectrumRT = FastFourierTransform(Graph, Ns, HeightSpectrumRT, FFTParameters, false);
            DisplaceXSpectrumRT = FastFourierTransform(Graph, Ns, DisplaceXSpectrumRT, FFTParameters, false);
            DisplaceYSpectrumRT = FastFourierTransform(Graph, Ns, DisplaceYSpectrumRT, FFTParameters, false);
        }
        CreateDisplacement(Graph, HeightSpectrumRT, DisplaceXSpectrumRT, DisplaceYSpectrumRT, OutDisplaceRT);
        CreateNormalFoam(Graph, OutDisplaceRT, FFTParameters, OutNormalRT, OutFoamRT);
    }

    UFourierTransformOceanComponent::UFourierTransformOceanComponent()
    {
        int32 N = glm::pow(2, FFTPow);
        GaussianRandomTexture = std::shared_ptr<UTexture2D>(UTexture2D::CreateTransient("GaussianRandomRT", N, N, EPixelFormat::PF_R16G16F));
        DisplaceTexture = std::shared_ptr<UTexture2D>(UTexture2D::CreateTransient("DisplaceTexture", N, N, EPixelFormat::PF_R16G16F));
        NormalTexture = std::shared_ptr<UTexture2D>(UTexture2D::CreateTransient("NormalTexture", N, N, EPixelFormat::PF_R16G16F));
        FoamTexture = std::shared_ptr<UTexture2D>(UTexture2D::CreateTransient("FoamTexture", N, N, EPixelFormat::PF_R16G16F));
        PerlinNoise = Cast<UTexture2D>(GetContentManager()->GetTextureByPath("/Textures/PerlinNoiseTexture.nasset"));

        InitialTime = clock();

        ENQUEUE_RENDER_COMMAND(UFourierTransformOceanComponent_ctor)(
            [this](RenderGraph& Graph, RHICommandListImmediate& RHICmdList)
            {
                FFTParameters = RenderGraph::CreatePersistentUniformBuffer<FOceanFastFourierTransformParameters>("OceanFastFourierTransformParameters");
            });
    }

    void UFourierTransformOceanComponent::TickComponent(double DeltaTime)
    {
        UpdateHeightField();
    }

    void UFourierTransformOceanComponent::UpdateHeightField()
    {
        ENQUEUE_RENDER_COMMAND(AddRenderingNodeList)(
            [FFTPow=this->FFTPow,
             GaussianRandomRT=this->GaussianRandomTexture->GetResource()->GetTextureRDG(),
             FFTParameters=this->FFTParameters,
             DisplaceRT=this->DisplaceTexture->GetResource()->GetTextureRDG(),
             NormalRT=this->NormalTexture->GetResource()->GetTextureRDG(),
             FoamRT=this->FoamTexture->GetResource()->GetTextureRDG(),
             WindDirection=this->WindDirection,
             N=glm::pow(2, FFTPow),
             WindSpeed=this->WindSpeed,
             Amplitude=this->Amplitude,
             Time=(clock()-this->InitialTime)/1000.f]
            (RenderGraph& Graph, RHICommandListImmediate& RHICmdList)
            {
                FFTParameters->SetData<&FOceanFastFourierTransformParameters::WindDirection>(WindDirection);
                FFTParameters->SetData<&FOceanFastFourierTransformParameters::N>(N);
                FFTParameters->SetData<&FOceanFastFourierTransformParameters::WindSpeed>(WindSpeed);
                FFTParameters->SetData<&FOceanFastFourierTransformParameters::Amplitude>(Amplitude);
                FFTParameters->SetData<&FOceanFastFourierTransformParameters::Time>(Time);
                
                UpdateHeightField_RenderThread(Graph, FFTPow, GaussianRandomRT, FFTParameters, DisplaceRT, NormalRT, FoamRT);
            }
        );
    }

}