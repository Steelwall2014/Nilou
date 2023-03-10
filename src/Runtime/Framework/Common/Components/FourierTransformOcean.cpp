#include "FourierTransformOcean.h"
#include "RenderingThread.h"
#include "Common/BaseApplication.h"

namespace nilou {

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

    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFastFourierTransformParameters)
        SHADER_PARAMETER(vec2, WindDirection)
        SHADER_PARAMETER(uint32, N)
        SHADER_PARAMETER(float, WindSpeed)
        SHADER_PARAMETER(float, Amplitude)
        SHADER_PARAMETER(float, DisplacementTextureSize)
        SHADER_PARAMETER(float, Time)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFFTButterflyBlock)
        SHADER_PARAMETER(uint32, Ns)
    END_UNIFORM_BUFFER_STRUCT()

    UFourierTransformOceanComponent::UFourierTransformOceanComponent(AActor* Owner)
        : UPrimitiveComponent(Owner)
    {
        
    }

    class FFourierTransformOceanSceneProxy : public FPrimitiveSceneProxy
    {
    
    public:

        FFourierTransformOceanSceneProxy(UFourierTransformOceanComponent* Component)
            : FPrimitiveSceneProxy(Component)
            , SurfaceSize(Component->SurfaceSize)
            , WindDirection(Component->WindDirection)
            , WindSpeed(Component->WindSpeed)
            , FFTPow(Component->FFTPow)
            , Amplitude(Component->Amplitude)
            , DisplacementTextureSize(Component->DisplacementTextureSize)
            , InitialTime(clock())
        {
            PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FFourierTransformOceanSceneProxy::PreRenderCallback);
        }

        void PreRenderCallback(FDynamicRHI* RHICmdList, FScene* Scene)
        {
            auto CurrentTime = clock();
            FFTParameters->Data.Time = float(CurrentTime - InitialTime) / 1000.f;
            FFTParameters->UpdateUniformBuffer();
            CreateDisplacementSpectrum();
            for (int m = 1; m <= FFTPow; m++)
            {
                unsigned int Ns = pow(2, m - 1);
                FastFourierTransform(Ns, HeightSpectrumRT, true);
                FastFourierTransform(Ns, DisplaceXSpectrumRT, true);
                FastFourierTransform(Ns, DisplaceYSpectrumRT, true);
            }
            for (int m = 1; m <= FFTPow; m++)
            {
                unsigned int Ns = pow(2, m - 1);
                FastFourierTransform(Ns, HeightSpectrumRT, false);
                FastFourierTransform(Ns, DisplaceXSpectrumRT, false);
                FastFourierTransform(Ns, DisplaceYSpectrumRT, false);
            }
            CreateDisplacement();
            CreateNormalFoam();
        }

        void CreateDisplacementSpectrum()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanDisplacementSpectrumShader::StaticType, 0);
            FShaderInstance *DisplacementSpectrumShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(DisplacementSpectrumShader);

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
            FShaderInstance *FFTShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(FFTShader);

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
                "OutputRT", OutputRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);

            std::swap(InputRT, OutputRT);

        }

        void CreateDisplacement()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanDisplacementShader::StaticType, 0);
            FShaderInstance *DisplacementShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(DisplacementShader);

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

        }

        void CreateNormalFoam()
        {
            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();
            FShaderPermutationParameters PermutationParameters(&FOceanNormalFoamShader::StaticType, 0);
            FShaderInstance *NormalFoamShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(NormalFoamShader);

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
        }

        virtual void CreateRenderThreadResources() override
        {
            FPrimitiveSceneProxy::CreateRenderThreadResources();

            FDynamicRHI* RHICmdList = FDynamicRHI::GetDynamicRHI();

            N = glm::pow(2, FFTPow);
            group_num = N / 32;     // 这里的32是写死在glsl中的"local_size"
            GaussianRandomRT = RHICmdList->RHICreateTexture2D("GaussianRandomRT", EPixelFormat::PF_R16G16F, 1, N, N, nullptr);
            HeightSpectrumRT = RHICmdList->RHICreateTexture2D("HeightSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, nullptr);
            DisplaceXSpectrumRT = RHICmdList->RHICreateTexture2D("DisplaceXSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, nullptr);
            DisplaceYSpectrumRT = RHICmdList->RHICreateTexture2D("DisplaceYSpectrumRT", EPixelFormat::PF_R16G16F, 1, N, N, nullptr);
            OutputRT = RHICmdList->RHICreateTexture2D("OutputRT", EPixelFormat::PF_R16G16F, 1, N, N, nullptr);
            DisplaceRT = RHICmdList->RHICreateTexture2D("DisplaceRT", EPixelFormat::PF_R16G16B16A16F, FFTPow, N, N, nullptr);
            NormalRT = RHICmdList->RHICreateTexture2D("NormalRT", EPixelFormat::PF_R16G16B16A16F, FFTPow, N, N, nullptr);
            FoamRT = RHICmdList->RHICreateTexture2D("FoamRT", EPixelFormat::PF_R16F, FFTPow, N, N, nullptr);
            
            FFTParameters = CreateUniformBuffer<FOceanFastFourierTransformParameters>();
            FFTParameters->Data.DisplacementTextureSize = DisplacementTextureSize;
            FFTParameters->Data.Amplitude = Amplitude;
            FFTParameters->Data.N = N;
            FFTParameters->Data.WindDirection = WindDirection;
            FFTParameters->Data.WindSpeed = WindSpeed;
            FFTParameters->InitRHI();

            ButterflyBlock = CreateUniformBuffer<FOceanFFTButterflyBlock>();
            ButterflyBlock->InitRHI();
            
            FShaderPermutationParameters PermutationParameters(&FOceanGaussionSpectrumShader::StaticType, 0);
            FShaderInstance *GaussionSpectrumShader = GetContentManager()->GetGlobalShader(PermutationParameters);
            FRHIGraphicsPipelineState *PSO = RHICmdList->RHISetComputeShader(GaussionSpectrumShader);

            RHICmdList->RHISetShaderUniformBuffer(
                PSO, EPipelineStage::PS_Compute, 
                "FOceanFastFourierTransformParameters", FFTParameters->GetRHI());

            RHICmdList->RHISetShaderImage(
                PSO, EPipelineStage::PS_Compute,
                "GaussianRandomRT", GaussianRandomRT.get(), EDataAccessFlag::DA_WriteOnly);

            RHICmdList->RHIDispatch(group_num, group_num, 1);

        }

        virtual void DestroyRenderThreadResources() override
        {
            GetAppication()->GetPreRenderDelegate().Remove(PreRenderHandle);

            FPrimitiveSceneProxy::DestroyRenderThreadResources();
        }

        float SurfaceSize;

		vec2 WindDirection;

        float WindSpeed;

        uint32 FFTPow;

        float Amplitude;

        float DisplacementTextureSize;

        uint32 group_num;

        uint32 N;    // N = pow(2, FFTPow)

        clock_t InitialTime;

		RHITexture2DRef GaussianRandomRT;          // 高斯随机数
		RHITexture2DRef HeightSpectrumRT;          // 高度频谱
		RHITexture2DRef DisplaceXSpectrumRT;       // X偏移频谱
		RHITexture2DRef DisplaceYSpectrumRT;       // Y偏移频谱
		RHITexture2DRef DisplaceRT;                // 偏移纹理
		RHITexture2DRef OutputRT;                  // 临时储存输出纹理
		RHITexture2DRef NormalRT;                  // 法线
		RHITexture2DRef FoamRT;					   // 白沫

		RHITexture2DRef PerlinNoise;

        TUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters;
        TUniformBufferRef<FOceanFFTButterflyBlock> ButterflyBlock;

    private:

        FDelegateHandle PreRenderHandle;

    };


    FPrimitiveSceneProxy* UFourierTransformOceanComponent::CreateSceneProxy()
    {
        return new FFourierTransformOceanSceneProxy(this);
    }

}