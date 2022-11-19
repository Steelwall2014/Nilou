#include "SkyAtmosphereComponent.h"
#include "Common/World.h"

namespace nilou {

    const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
    const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

    const int SCATTERING_TEXTURE_R_SIZE = 32;
    const int SCATTERING_TEXTURE_MU_SIZE = 128;
    const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
    const int SCATTERING_TEXTURE_NU_SIZE = 8;

    const int SCATTERING_TEXTURE_WIDTH =
        SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
    const int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
    const int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

    const int IRRADIANCE_TEXTURE_WIDTH = 64;
    const int IRRADIANCE_TEXTURE_HEIGHT = 16;

    const int NUM_SCATTERING_ORDERS = 4;

    DECLARE_GLOBAL_SHADER(FAtmosphereTransmittanceShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereTransmittanceShader, "/Shaders/SkyAtmosphere/atmosphere_transmittance_pre.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FAtmosphereDirectIrradianceShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereDirectIrradianceShader, "/Shaders/SkyAtmosphere/atmosphere_direct_irradiance_pre.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FAtmosphereScatteringShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereScatteringShader, "/Shaders/SkyAtmosphere/atmosphere_scattering_pre.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FAtmosphereScatteringDensityShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereScatteringDensityShader, "/Shaders/SkyAtmosphere/atmosphere_scattering_density_pre.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FAtmosphereIndirectIrradianceShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereIndirectIrradianceShader, "/Shaders/SkyAtmosphere/atmosphere_indirect_irradiance_pre.comp", EShaderFrequency::SF_Compute, Global)

    DECLARE_GLOBAL_SHADER(FAtmosphereMultiScatteringShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereMultiScatteringShader, "/Shaders/SkyAtmosphere/atmosphere_multiscattering_pre.comp", EShaderFrequency::SF_Compute, Global)
    

    USkyAtmosphereComponent::USkyAtmosphereComponent(AActor *InOwner)
        : USceneComponent(InOwner)
        , SolarIrradiance({ 1.474000, 1.850400, 1.911980 })
        , SunAngularRadius(0.004675)
        , BottomRadius(6360.0)
        , TopRadius(6420.0)
        , RayleighDensity({{ 0,0,0,0,0 }, { 0,1,-0.125,0,0 }})
        , RayleighScattering({ 0.005802,0.013558,0.033100 })
        , MieDensity({{ 0,0,0,0,0 }, { 0,1,-0.833333,0,0 }})
        , MieScattering({0.003996, 0.003996, 0.003996})
        , MieExtinction({0.004440, 0.004440, 0.004440})
        , MiePhaseFunction_g(0.8)
        , AbsorptionDensity({{ 25,0,0,0.066667,-0.666667 }, { 0,0,0,-0.066667,2.666667 }})
        , AbsorptionExtinction({ 0.000650, 0.001881, 0.000085 })
        , GroundAlbedo({0.1,0.1,0.1})
        , Mu_s_Min(-0.207912)
    {
        
    }

    void USkyAtmosphereComponent::CreateRenderState()
    {
        USceneComponent::CreateRenderState();
        if (IsRegistered())
        {
		    SkyAtmosphereSceneProxy = new FSkyAtmosphereSceneProxy(this);
		    GetWorld()->Scene->AddSkyAtmosphere(SkyAtmosphereSceneProxy);
        }
    }

    void USkyAtmosphereComponent::DestroyRenderState()
    {
        if (IsRegistered())
        {
		    GetWorld()->Scene->RemoveSkyAtmosphere(SkyAtmosphereSceneProxy);
        }
        USceneComponent::DestroyRenderState();
    }

    ShaderDensityProfile TranslateDensityProfile(const DensityProfile &profile)
    {
        ShaderDensityProfile shader_profile;
        shader_profile.layers[0].constant_term = profile.layers[0].constant_term;
        shader_profile.layers[0].linear_term = profile.layers[0].linear_term;
        shader_profile.layers[0].exp_term = profile.layers[0].exp_term;
        shader_profile.layers[0].exp_scale = profile.layers[0].exp_scale;
        shader_profile.layers[0].width = profile.layers[0].width;
        shader_profile.layers[1].constant_term = profile.layers[1].constant_term;
        shader_profile.layers[1].linear_term = profile.layers[1].linear_term;
        shader_profile.layers[1].exp_term = profile.layers[1].exp_term;
        shader_profile.layers[1].exp_scale = profile.layers[1].exp_scale;
        shader_profile.layers[1].width = profile.layers[1].width;
        return shader_profile;
    }

    #define FROM_COMPONENNT_TO_PROXY(MemberName) Set##MemberName(InComponent->Get##MemberName());

    FSkyAtmosphereSceneProxy::FSkyAtmosphereSceneProxy(const USkyAtmosphereComponent* InComponent)
    {
        AtmosphereParameters = CreateUniformBuffer<ShaderAtmosphereParametersBlock>();
		FROM_COMPONENNT_TO_PROXY(SolarIrradiance)
		FROM_COMPONENNT_TO_PROXY(SunAngularRadius)
		FROM_COMPONENNT_TO_PROXY(BottomRadius)
		FROM_COMPONENNT_TO_PROXY(TopRadius)
        SetRayleighDensity(TranslateDensityProfile(InComponent->GetRayleighDensity()));
		FROM_COMPONENNT_TO_PROXY(RayleighScattering)
        SetMieDensity(TranslateDensityProfile(InComponent->GetMieDensity()));
		FROM_COMPONENNT_TO_PROXY(MieScattering)
		FROM_COMPONENNT_TO_PROXY(MieExtinction)
		FROM_COMPONENNT_TO_PROXY(MiePhaseFunction_g)
        SetAbsorptionDensity(TranslateDensityProfile(InComponent->GetAbsorptionDensity()));
		FROM_COMPONENNT_TO_PROXY(AbsorptionExtinction)
		FROM_COMPONENNT_TO_PROXY(GroundAlbedo)
		FROM_COMPONENNT_TO_PROXY(Mu_s_Min)
        AtmosphereParameters->InitRHI();

        ScatteringOrderParameter = CreateUniformBuffer<ScatteringOrderBlock>();

        TransmittanceLUT = GDynamicRHI->RHICreateTexture2D(
            "SkyAtmosphere TransmittanceLUT", EPixelFormat::PF_R32G32B32A32F, 1, 
            TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, nullptr);

        IrradianceLUT = GDynamicRHI->RHICreateTexture2D(
            "SkyAtmosphere IrradianceLUT", EPixelFormat::PF_R32G32B32A32F, 1, 
            IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, nullptr);

        DeltaScatteringRayleighLUT = GDynamicRHI->RHICreateTexture3D(
            "SkyAtmosphere SingleScatteringRayleighLUT", EPixelFormat::PF_R32G32B32A32F, 1, 
            SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);

        SingleScatteringMieLUT = GDynamicRHI->RHICreateTexture3D(
            "SkyAtmosphere SingleScatteringMieLUT", EPixelFormat::PF_R32G32B32A32F, 1, 
            SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);

        MultiScatteringLUT = GDynamicRHI->RHICreateTexture3D(
            "SkyAtmosphere MultiScatteringLUT", EPixelFormat::PF_R32G32B32A32F, 1, 
            SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);

        ScatteringDensityLUT = GDynamicRHI->RHICreateTexture3D(
            "SkyAtmosphere ScatteringDensityLUT", EPixelFormat::PF_R32G32B32A32F, 1,
            SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);
        ScatteringOrderParameter->InitRHI();
        DispatchPrecompute();
    }

    #undef FROM_COMPONENNT_TO_PROXY

    void FSkyAtmosphereSceneProxy::DispatchPrecompute()
    {
        DispatchTransmittancePass();
        DispatchDirectIrradiancePass();
        DispatchScatteringPass();
        for (int scattering_order = 2; scattering_order <= NUM_SCATTERING_ORDERS; ++scattering_order) 
        {
            ScatteringOrderParameter->Data.ScatteringOrder = scattering_order;
            ScatteringOrderParameter->UpdateUniformBuffer();
            DispatchScatteringDensityPass();
            DispatchIndirectIrradiancePass();
            DispatchMultiScatteringPass();
        }
    }
    
    void FSkyAtmosphereSceneProxy::DispatchTransmittancePass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereTransmittanceShader::StaticType, 0);
        FShaderInstance *TransmittanceShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(TransmittanceShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "TransmittanceLUT", TransmittanceLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHIDispatch(TRANSMITTANCE_TEXTURE_WIDTH / 8, TRANSMITTANCE_TEXTURE_HEIGHT / 8, 1);
    }

    void FSkyAtmosphereSceneProxy::DispatchDirectIrradiancePass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereDirectIrradianceShader::StaticType, 0);
        FShaderInstance *DirectIrradianceShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(DirectIrradianceShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "IrradianceLUT", IrradianceLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "TransmittanceLUT", FRHISampler(TransmittanceLUT));
        GDynamicRHI->RHIDispatch(IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);
    }

    void FSkyAtmosphereSceneProxy::DispatchScatteringPass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereScatteringShader::StaticType, 0);
        FShaderInstance *ScatteringShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(ScatteringShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "TransmittanceLUT", FRHISampler(TransmittanceLUT));
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "SingleScatteringRayleighLUT", DeltaScatteringRayleighLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "SingleScatteringMieLUT", SingleScatteringMieLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "MultiScatteringLUT", MultiScatteringLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
    }

    void FSkyAtmosphereSceneProxy::DispatchScatteringDensityPass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereScatteringDensityShader::StaticType, 0);
        FShaderInstance *ScatteringDensityShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(ScatteringDensityShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "ScatteringOrderBlock", ScatteringOrderParameter->GetRHI());
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "ScatteringDensityLUT", ScatteringDensityLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "TransmittanceLUT", FRHISampler(TransmittanceLUT));
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "SingleScatteringRayleighLUT", FRHISampler(DeltaScatteringRayleighLUT));
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "SingleScatteringMieLUT", FRHISampler(SingleScatteringMieLUT));
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "IrradianceLUT", FRHISampler(IrradianceLUT));
        GDynamicRHI->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
    }

    void FSkyAtmosphereSceneProxy::DispatchIndirectIrradiancePass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereIndirectIrradianceShader::StaticType, 0);
        FShaderInstance *IndirectIrradianceShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(IndirectIrradianceShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "ScatteringOrderBlock", ScatteringOrderParameter->GetRHI());
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "IrradianceLUT", IrradianceLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "SingleScatteringRayleighLUT", FRHISampler(DeltaScatteringRayleighLUT));
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "SingleScatteringMieLUT", FRHISampler(SingleScatteringMieLUT));
        GDynamicRHI->RHIDispatch(IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);
    }

    void FSkyAtmosphereSceneProxy::DispatchMultiScatteringPass()
    {
        FShaderPermutationParameters PermutationParameters(&FAtmosphereMultiScatteringShader::StaticType, 0);
        FShaderInstance *MultiScatteringShader = GetGlobalShaderInstance2(PermutationParameters);
        FRHIGraphicsPipelineState *PSO = GDynamicRHI->RHISetComputeShader(MultiScatteringShader);
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "AtmosphereParametersBlock", AtmosphereParameters->GetRHI());
        GDynamicRHI->RHISetShaderUniformBuffer(PSO, EPipelineStage::PS_Compute, "ScatteringOrderBlock", ScatteringOrderParameter->GetRHI());
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "DeltaScatteringLUT", DeltaScatteringRayleighLUT.get(), EDataAccessFlag::DA_WriteOnly);
        GDynamicRHI->RHISetShaderImage(PSO, EPipelineStage::PS_Compute, "MultiScatteringLUT", MultiScatteringLUT.get(), EDataAccessFlag::DA_ReadWrite);
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "TransmittanceLUT", FRHISampler(TransmittanceLUT));
        GDynamicRHI->RHISetShaderSampler(PSO, EPipelineStage::PS_Compute, "ScatteringDensityLUT", FRHISampler(ScatteringDensityLUT));
        GDynamicRHI->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
    }

}