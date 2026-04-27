#include "Components/FourierTransformOcean.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "GenerateMips.h"
#include "Engine/Texture2D.h"
#include "OceanGaussianSpectrum.generated.h"
#include "OceanDisplacementSpectrum.generated.h"
#include "OceanFastFourierTransform.generated.h"
#include "OceanCreateDisplacement.generated.h"
#include "OceanCreateNormalFoam.generated.h"
#include "OceanFastFourierTransformParameters.generated.h"

namespace nilou {

    constexpr int MAX_RENDERING_NODES = 500;

    DECLARE_GLOBAL_SHADER(FOceanGaussionSpectrumShader)
    IMPLEMENT_SHADER_TYPE(FOceanGaussionSpectrumShader, "/Shaders/Private/FastFourierTransformOcean/OceanGaussianSpectrum.slang", "Main", EShaderFrequency::Compute);

    DECLARE_GLOBAL_SHADER(FOceanDisplacementSpectrumShader)
    IMPLEMENT_SHADER_TYPE(FOceanDisplacementSpectrumShader, "/Shaders/Private/FastFourierTransformOcean/OceanDisplacementSpectrum.slang", "Main", EShaderFrequency::Compute);

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
    IMPLEMENT_SHADER_TYPE(FOceanFastFourierTransformShader, "/Shaders/Private/FastFourierTransformOcean/OceanFastFourierTransform.slang", "Main", EShaderFrequency::Compute);

    DECLARE_GLOBAL_SHADER(FOceanDisplacementShader)
    IMPLEMENT_SHADER_TYPE(FOceanDisplacementShader, "/Shaders/Private/FastFourierTransformOcean/OceanCreateDisplacement.slang", "Main", EShaderFrequency::Compute);

    DECLARE_GLOBAL_SHADER(FOceanNormalFoamShader)
    IMPLEMENT_SHADER_TYPE(FOceanNormalFoamShader, "/Shaders/Private/FastFourierTransformOcean/OceanCreateNormalFoam.slang", "Main", EShaderFrequency::Compute);

    static void CreateGaussianRandom(RenderGraph& Graph, TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters, RDGTexture* OutGaussianRandomRT)
    {
        int32 group_num = OutGaussianRandomRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanGaussionSpectrumShader::StaticType, 0);
        RHIShader *GaussionSpectrumShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(GaussionSpectrumShader));

        auto Params = Graph.CreateParameterBlock<shader::FOceanGaussianSpectrumTextures>("GaussianSpectrum ParamBlock");
        Params->GaussianRandomRT = OutGaussianRandomRT->GetDefaultView();
        Graph.UpdateParameterBlock(Params);

        RDGPassDesc PassDesc{"CreateGaussionSpectrum"};
        Graph.AddComputePass(
            PassDesc,
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(Params);
                Pass->AddParameterBlock(FFTParameters.GetReference());
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                int32 OceanFFTParamsSetIndex = PipelineLayout->GetSetIndex("OceanFFTParameters");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {{ParamsSetIndex, Params->GetDescriptorSet()->GetRHI()}, {OceanFFTParamsSetIndex, FFTParameters->GetDescriptorSet()->GetRHI()}},
                    EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );
    }

    static std::tuple<RDGTexture*, RDGTexture*, RDGTexture*> CreateDisplacementSpectrum(RenderGraph& Graph, RDGTexture* GaussianRandomRT, TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters)
    {
        int32 group_num = GaussianRandomRT->Desc.SizeX / 32;
        RDGTexture* HeightSpectrumRT = Graph.CreateTexture("FastFourierTransform HeightSpectrumRT", GaussianRandomRT->Desc);
        RDGTexture* DisplaceXSpectrumRT = Graph.CreateTexture("FastFourierTransform DisplaceXSpectrumRT", GaussianRandomRT->Desc);
        RDGTexture* DisplaceYSpectrumRT = Graph.CreateTexture("FastFourierTransform DisplaceYSpectrumRT", GaussianRandomRT->Desc);

        FShaderPermutationParameters PermutationParameters(&FOceanDisplacementSpectrumShader::StaticType, 0);
        RHIShader *DisplacementSpectrumShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(DisplacementSpectrumShader));

        auto Params = Graph.CreateParameterBlock<shader::FOceanDisplacementSpectrumParameters>("DisplacementSpectrum ParamBlock");
        Params->GaussianRandomRT = GaussianRandomRT->GetDefaultView();
        Params->HeightSpectrumRT = HeightSpectrumRT->GetDefaultView();
        Params->DisplaceXSpectrumRT = DisplaceXSpectrumRT->GetDefaultView();
        Params->DisplaceYSpectrumRT = DisplaceYSpectrumRT->GetDefaultView();
        Graph.UpdateParameterBlock(Params);

        RDGPassDesc PassDesc{"CreateDisplacementSpectrum"};
        Graph.AddComputePass(
            PassDesc,
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(Params);
                Pass->AddParameterBlock(FFTParameters.GetReference());
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                int32 OceanFFTParamsSetIndex = PipelineLayout->GetSetIndex("OceanFFTParameters");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {{ParamsSetIndex, Params->GetDescriptorSet()->GetRHI()}, {OceanFFTParamsSetIndex, FFTParameters->GetDescriptorSet()->GetRHI()}},
                    EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        return { HeightSpectrumRT, DisplaceXSpectrumRT, DisplaceYSpectrumRT };
    }

    static RDGTexture* FastFourierTransform(RenderGraph& Graph, uint32 Ns, RDGTexture* InputRT, TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters, bool bHorizontalPass)
    {
        int32 group_num = InputRT->Desc.SizeX / 32;
        FOceanFastFourierTransformShader::FPermutationDomain PermutationVector;
        PermutationVector.Set<FOceanFastFourierTransformShader::FDimensionHorizontalPass>(bHorizontalPass);
        FShaderPermutationParameters PermutationParameters(&FOceanFastFourierTransformShader::StaticType, PermutationVector.ToDimensionValueId());
        RHIShader *FFTShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(FFTShader));

        RDGTexture* OutputRT = Graph.CreateTexture("FastFourierTransform OutputRT", InputRT->Desc);

        auto Params = Graph.CreateParameterBlock<shader::FOceanFastFourierTransformTextures>("FFT Textures ParamBlock");
        Params->InputRT = InputRT->GetDefaultView();
        Params->OutputRT = OutputRT->GetDefaultView();
        Graph.UpdateParameterBlock(Params);

        auto ButterflyBlock = Graph.CreateParameterBlock<shader::FOceanFFTButterflyBlock>("FFT ButterflyBlock ParamBlock");
        ButterflyBlock->GetNonOpaqueFields().Ns = Ns;
        ButterflyBlock->GetNonOpaqueFields().HorizontalPass = bHorizontalPass;
        Graph.UpdateParameterBlock(ButterflyBlock);

        RDGPassDesc PassDesc{"FastFourierTransform"};
        Graph.AddComputePass(
            PassDesc,
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(Params);
                Pass->AddParameterBlock(ButterflyBlock);
                Pass->AddParameterBlock(FFTParameters.GetReference());
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                int32 ButterflyBlockSetIndex = PipelineLayout->GetSetIndex("OceanFFTButterflyBlock");
                int32 OceanFFTParamsSetIndex = PipelineLayout->GetSetIndex("OceanFFTParameters");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {{ParamsSetIndex, Params->GetDescriptorSet()->GetRHI()}, {ButterflyBlockSetIndex, ButterflyBlock->GetDescriptorSet()->GetRHI()}, {OceanFFTParamsSetIndex, FFTParameters->GetDescriptorSet()->GetRHI()}},
                    EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        return OutputRT;
    }

    static void CreateDisplacement(RenderGraph& Graph, RDGTexture* HeightSpectrumRT, RDGTexture* DisplaceXSpectrumRT, RDGTexture* DisplaceYSpectrumRT, RDGTexture* OutDisplaceRT)
    {
        int32 group_num = HeightSpectrumRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanDisplacementShader::StaticType, 0);
        RHIShader *DisplacementShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(DisplacementShader));

        auto Params = Graph.CreateParameterBlock<shader::FOceanCreateDisplacementParameters>("CreateDisplacement ParamBlock");
        Params->HeightSpectrumRT = HeightSpectrumRT->GetDefaultView();
        Params->DisplaceXSpectrumRT = DisplaceXSpectrumRT->GetDefaultView();
        Params->DisplaceYSpectrumRT = DisplaceYSpectrumRT->GetDefaultView();
        Params->DisplaceRT = OutDisplaceRT->GetDefaultView();
        Graph.UpdateParameterBlock(Params);

        RDGPassDesc PassDesc{"CreateDisplacement"};
        Graph.AddComputePass(
            PassDesc,
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(Params);
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {{ParamsSetIndex, Params->GetDescriptorSet()->GetRHI()}},
                    EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        FGenerateMips::Execute(Graph, OutDisplaceRT);
    }

    static void CreateNormalFoam(RenderGraph& Graph, RDGTexture* DisplaceRT, TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters, RDGTexture* OutNormalRT, RDGTexture* OutFoamRT)
    {
        int32 group_num = DisplaceRT->Desc.SizeX / 32;
        FShaderPermutationParameters PermutationParameters(&FOceanNormalFoamShader::StaticType, 0);
        RHIShader *NormalFoamShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(NormalFoamShader));

        auto Params = Graph.CreateParameterBlock<shader::FOceanCreateNormalFoamParameters>("CreateNormalFoam ParamBlock");
        Params->DisplaceRT = DisplaceRT->GetDefaultView();
        Params->NormalRT = OutNormalRT->GetDefaultView();
        Params->FoamRT = OutFoamRT->GetDefaultView();
        Graph.UpdateParameterBlock(Params);

        RDGPassDesc PassDesc{"CreateNormalFoam"};
        Graph.AddComputePass(
            PassDesc,
            [=](FRDGPass* Pass)
            {
                Pass->AddParameterBlock(Params);
                Pass->AddParameterBlock(FFTParameters.GetReference());
            },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                auto PipelineLayout = PSO->GetPipelineLayout();
                int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                int32 OceanFFTParamsSetIndex = PipelineLayout->GetSetIndex("OceanFFTParameters");
                RHICmdList.BindDescriptorSets(
                    PipelineLayout,
                    {{ParamsSetIndex, Params->GetDescriptorSet()->GetRHI()}, {OceanFFTParamsSetIndex, FFTParameters->GetDescriptorSet()->GetRHI()}},
                    EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(group_num, group_num, 1);
            }
        );

        FGenerateMips::Execute(Graph, OutNormalRT);
    }

    static void UpdateHeightField_RenderThread(
        RenderGraph& Graph, int32 FFTPow, RDGTexture* GaussianRandomRT, TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters,
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
        PerlinNoise = LoadObject<UTexture2D>("/Textures/PerlinNoiseTexture.PerlinNoiseTexture");

        InitialTime = clock();

        ENQUEUE_RENDER_COMMAND(UFourierTransformOceanComponent_ctor)(
            [this](RenderGraph&)
            {
                FFTParameters = RenderGraph::CreatePooledParameterBlock<shader::FOceanFastFourierTransformParameters>("OceanFastFourierTransformParameters");
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
             DisplacementTextureSize=this->DisplacementTextureSize,
             Time=(clock()-this->InitialTime)/1000.f]
            (RenderGraph& Graph)
            {
                FFTParameters->WindDirection = WindDirection;
                FFTParameters->N = N;
                FFTParameters->WindSpeed = WindSpeed;
                FFTParameters->Amplitude = Amplitude;
                FFTParameters->DisplacementTextureSize = DisplacementTextureSize;
                FFTParameters->Time = Time;
                Graph.UpdateParameterBlock(FFTParameters.GetReference());
                UpdateHeightField_RenderThread(Graph, FFTPow, GaussianRandomRT, FFTParameters, DisplaceRT, NormalRT, FoamRT);
            }
        );
    }

}
