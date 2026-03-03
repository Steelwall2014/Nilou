#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"

#include "RenderingThread.h"
#include "Shader.h"
#include "ShaderInstance.h"
// #include "ShaderParameterBlock.h"

#include "atmosphere_transmittance_pre.generated.h"
#include "atmosphere_direct_irradiance_pre.generated.h"
#include "atmosphere_scattering_pre.generated.h"
#include "atmosphere_scattering_density_pre.generated.h"
#include "atmosphere_indirect_irradiance_pre.generated.h"
#include "atmosphere_multiscattering_pre.generated.h"

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
    IMPLEMENT_SHADER_TYPE(FAtmosphereTransmittanceShader, "/Shaders/Private/SkyAtmosphere/atmosphere_transmittance_pre.slang", "Main", EShaderFrequency::Compute)

    DECLARE_GLOBAL_SHADER(FAtmosphereDirectIrradianceShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereDirectIrradianceShader, "/Shaders/Private/SkyAtmosphere/atmosphere_direct_irradiance_pre.slang", "Main", EShaderFrequency::Compute)

    DECLARE_GLOBAL_SHADER(FAtmosphereScatteringShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereScatteringShader, "/Shaders/Private/SkyAtmosphere/atmosphere_scattering_pre.slang", "Main", EShaderFrequency::Compute)

    DECLARE_GLOBAL_SHADER(FAtmosphereScatteringDensityShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereScatteringDensityShader, "/Shaders/Private/SkyAtmosphere/atmosphere_scattering_density_pre.slang", "Main", EShaderFrequency::Compute)

    DECLARE_GLOBAL_SHADER(FAtmosphereIndirectIrradianceShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereIndirectIrradianceShader, "/Shaders/Private/SkyAtmosphere/atmosphere_indirect_irradiance_pre.slang", "Main", EShaderFrequency::Compute)

    DECLARE_GLOBAL_SHADER(FAtmosphereMultiScatteringShader)
    IMPLEMENT_SHADER_TYPE(FAtmosphereMultiScatteringShader, "/Shaders/Private/SkyAtmosphere/atmosphere_multiscattering_pre.slang", "Main", EShaderFrequency::Compute)
    

    USkyAtmosphereComponent::USkyAtmosphereComponent()
        : SolarIrradiance(FVector3f( 1 ))
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

    FSkyAtmosphereSceneProxy::FSkyAtmosphereSceneProxy(const USkyAtmosphereComponent* InComponent)
    {
        ATMOSPHERE.solar_irradiance = InComponent->SolarIrradiance;
        ATMOSPHERE.sun_angular_radius = InComponent->SunAngularRadius;
        ATMOSPHERE.bottom_radius = InComponent->BottomRadius;
        ATMOSPHERE.top_radius = InComponent->TopRadius;
        ATMOSPHERE.rayleigh_density = InComponent->RayleighDensity;
        ATMOSPHERE.rayleigh_scattering = InComponent->RayleighScattering;
        ATMOSPHERE.mie_density = InComponent->MieDensity;
        ATMOSPHERE.mie_scattering = InComponent->MieScattering;
        ATMOSPHERE.mie_extinction = InComponent->MieExtinction;
        ATMOSPHERE.mie_phase_function_g = InComponent->MiePhaseFunction_g;
        ATMOSPHERE.absorption_density = InComponent->AbsorptionDensity;
        ATMOSPHERE.absorption_extinction = InComponent->AbsorptionExtinction;
        ATMOSPHERE.ground_albedo = InComponent->GroundAlbedo;
        ATMOSPHERE.mu_s_min = InComponent->Mu_s_Min;
        ENQUEUE_RENDER_COMMAND(FSkyAtmosphereSceneProxyConstructor)([this](RenderGraph& Graph) {

            AtmosphereParameters = RenderGraph::CreatePooledUniformBuffer("AtmosphereParameters", &ATMOSPHERE.GetNonOpaqueFields());
            ATMOSPHERE.AutomaticallyIntroducedUniformBuffer = AtmosphereParameters.GetReference();
            Graph.QueueBufferUpload(AtmosphereParameters.GetReference(), &ATMOSPHERE.GetNonOpaqueFields(), sizeof(ATMOSPHERE.GetNonOpaqueFields()));

            AtmosphereParametersDescriptorSet = RenderGraph::CreatePooledDescriptorSet("AtmosphereParametersDescriptorSet", ATMOSPHERE);

            RDGTextureDesc Desc;
            Desc.TextureType = ETextureDimension::Texture2D;
            Desc.Format = EPixelFormat::PF_R32G32B32A32F;
            Desc.NumMips = 1;
            Desc.SizeX = TRANSMITTANCE_TEXTURE_WIDTH;
            Desc.SizeY = TRANSMITTANCE_TEXTURE_HEIGHT;
            TransmittanceLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere TransmittanceLUT", Desc);

            Desc.SizeX = IRRADIANCE_TEXTURE_WIDTH;
            Desc.SizeY = IRRADIANCE_TEXTURE_HEIGHT;
            IrradianceLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere IrradianceLUT", Desc);

            Desc.TextureType = ETextureDimension::Texture3D;
            Desc.SizeX = SCATTERING_TEXTURE_WIDTH;
            Desc.SizeY = SCATTERING_TEXTURE_HEIGHT;
            Desc.SizeZ = SCATTERING_TEXTURE_DEPTH;
            DeltaScatteringRayleighLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere SingleScatteringRayleighLUT", Desc);
            SingleScatteringMieLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere SingleScatteringMieLUT", Desc);
            MultiScatteringLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere MultiScatteringLUT", Desc);
            ScatteringDensityLUT = RenderGraph::CreatePooledTexture("SkyAtmosphere ScatteringDensityLUT", Desc);

            DispatchPrecompute();
        });
    }

    void FSkyAtmosphereSceneProxy::DispatchPrecompute()
    {
        DispatchTransmittancePass();
        DispatchDirectIrradiancePass();
        DispatchScatteringPass();
        for (int scattering_order = 2; scattering_order <= NUM_SCATTERING_ORDERS; ++scattering_order) 
        {
            DispatchScatteringDensityPass(scattering_order);
            DispatchIndirectIrradiancePass(scattering_order);
            DispatchMultiScatteringPass();
        }
    }
    
    void FSkyAtmosphereSceneProxy::DispatchTransmittancePass()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereTransmittanceShader::StaticType, 0);
        RHIShader *TransmittanceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(TransmittanceShader));
        TParameterBlock<FAtmosphereTransmittanceParameters> Params;
        Params.TransmittanceLUT = TransmittanceLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("TransmittanceShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchTransmittancePass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(TRANSMITTANCE_TEXTURE_WIDTH / 8, TRANSMITTANCE_TEXTURE_HEIGHT / 8, 1);
            });
    }

    void FSkyAtmosphereSceneProxy::DispatchDirectIrradiancePass()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereDirectIrradianceShader::StaticType, 0);
        RHIShader *DirectIrradianceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(DirectIrradianceShader));
        TParameterBlock<FAtmosphereDirectIrradianceParameters> Params;
        Params.IrradianceLUT = IrradianceLUT->GetDefaultView();
        Params.TransmittanceLUT = TransmittanceLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("DirectIrradianceShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchDirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);
            });
    }

    void FSkyAtmosphereSceneProxy::DispatchScatteringPass()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereScatteringShader::StaticType, 0);
        RHIShader *ScatteringShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(ScatteringShader));
        TParameterBlock<FAtmosphereScatteringParameters> Params;
        Params.TransmittanceLUT = TransmittanceLUT->GetDefaultView();
        Params.SingleScatteringRayleighLUT = DeltaScatteringRayleighLUT->GetDefaultView();
        Params.SingleScatteringMieLUT = SingleScatteringMieLUT->GetDefaultView();
        Params.MultiScatteringLUT = MultiScatteringLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("ScatteringShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchScatteringPass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
            });
    }

    void FSkyAtmosphereSceneProxy::DispatchScatteringDensityPass(int32 scattering_order)
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereScatteringDensityShader::StaticType, 0);
        RHIShader *ScatteringDensityShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(ScatteringDensityShader));
        TParameterBlock<FAtmosphereScatteringDensityParameters> Params;
        Params.ScatteringDensityLUT = ScatteringDensityLUT->GetDefaultView();
        Params.TransmittanceLUT = TransmittanceLUT->GetDefaultView();
        Params.SingleScatteringRayleighLUT = DeltaScatteringRayleighLUT->GetDefaultView();
        Params.SingleScatteringMieLUT = SingleScatteringMieLUT->GetDefaultView();
        Params.IrradianceLUT = IrradianceLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("ScatteringDensityShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchScatteringDensityPass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.PushConstants(PSO->GetPipelineLayout(), EShaderStage::Compute, 0, 4, &scattering_order);
                RHICmdList.DispatchCompute(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
            });
    }

    void FSkyAtmosphereSceneProxy::DispatchIndirectIrradiancePass(int32 scattering_order)
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereIndirectIrradianceShader::StaticType, 0);
        RHIShader *IndirectIrradianceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(IndirectIrradianceShader));
        TParameterBlock<FAtmosphereIndirectIrradianceParameters> Params;
        Params.IrradianceLUT = IrradianceLUT->GetDefaultView();
        Params.SingleScatteringRayleighLUT = DeltaScatteringRayleighLUT->GetDefaultView();
        Params.SingleScatteringMieLUT = SingleScatteringMieLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("IndirectIrradianceShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchIndirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.PushConstants(PSO->GetPipelineLayout(), EShaderStage::Compute, 0, 4, &scattering_order);
                RHICmdList.DispatchCompute(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
            });
    }

    void FSkyAtmosphereSceneProxy::DispatchMultiScatteringPass()
    {
        RenderGraph& Graph = FRenderingThread::GetRenderGraph();
        FShaderPermutationParameters PermutationParameters(&FAtmosphereMultiScatteringShader::StaticType, 0);
        RHIShader *MultiScatteringShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(MultiScatteringShader));
        TParameterBlock<FAtmosphereMultiScatteringParameters> Params;
        Params.DeltaScatteringLUT = DeltaScatteringRayleighLUT->GetDefaultView();
        Params.MultiScatteringLUT = MultiScatteringLUT->GetDefaultView();
        Params.TransmittanceLUT = TransmittanceLUT->GetDefaultView();
        Params.ScatteringDensityLUT = ScatteringDensityLUT->GetDefaultView();
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("MultiScatteringShader DescriptorSet", Params);
        RDGPassDesc PassDesc{"DispatchIndirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { AtmosphereParametersDescriptorSet.GetReference(), DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
            });
    }

}