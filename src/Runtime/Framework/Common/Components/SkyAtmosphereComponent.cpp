#include "SkyAtmosphereComponent.h"
#include "Common/World.h"

#include "RenderingThread.h"
#include "Common/ContentManager.h"
#include "Shader.h"
#include "ShaderInstance.h"

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
    

    USkyAtmosphereComponent::USkyAtmosphereComponent()
        : SolarIrradiance(vec3( 1 ))
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

    FSkyAtmosphereSceneProxy::FSkyAtmosphereSceneProxy(const USkyAtmosphereComponent* InComponent)
        : SolarIrradiance(InComponent->GetSolarIrradiance())
        , SunAngularRadius(InComponent->GetSunAngularRadius())
        , BottomRadius(InComponent->GetBottomRadius())
        , TopRadius(InComponent->GetTopRadius())
        , RayleighDensity(TranslateDensityProfile(InComponent->GetRayleighDensity()))
        , RayleighScattering(InComponent->GetRayleighScattering())
        , MieDensity(TranslateDensityProfile(InComponent->GetMieDensity()))
        , MieScattering(InComponent->GetMieScattering())
        , MieExtinction(InComponent->GetMieExtinction())
        , MiePhaseFunction_g(InComponent->GetMiePhaseFunction_g())
        , AbsorptionDensity(TranslateDensityProfile(InComponent->GetAbsorptionDensity()))
        , AbsorptionExtinction(InComponent->GetAbsorptionExtinction())
        , GroundAlbedo(InComponent->GetGroundAlbedo())
        , Mu_s_Min(InComponent->GetMu_s_Min())
    {

        ENQUEUE_RENDER_COMMAND(FSkyAtmosphereSceneProxyConstructor)([this](RenderGraph&) {

            AtmosphereParameters = RenderGraph::CreateExternalUniformBuffer<ShaderAtmosphereParametersBlock>("", nullptr);

            RDGTextureDesc Desc;
            Desc.TextureType = ETextureDimension::Texture2D;
            Desc.Format = EPixelFormat::PF_R32G32B32A32F;
            Desc.NumMips = 1;
            Desc.SizeX = TRANSMITTANCE_TEXTURE_WIDTH;
            Desc.SizeY = TRANSMITTANCE_TEXTURE_HEIGHT;
            TransmittanceLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere TransmittanceLUT", Desc);

            Desc.SizeX = IRRADIANCE_TEXTURE_WIDTH;
            Desc.SizeY = IRRADIANCE_TEXTURE_HEIGHT;
            IrradianceLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere IrradianceLUT", Desc);

            Desc.TextureType = ETextureDimension::Texture3D;
            Desc.SizeX = SCATTERING_TEXTURE_WIDTH;
            Desc.SizeY = SCATTERING_TEXTURE_HEIGHT;
            Desc.SizeZ = SCATTERING_TEXTURE_DEPTH;
            DeltaScatteringRayleighLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere SingleScatteringRayleighLUT", Desc);
            SingleScatteringMieLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere SingleScatteringMieLUT", Desc);
            MultiScatteringLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere MultiScatteringLUT", Desc);
            ScatteringDensityLUT = RenderGraph::CreateExternalTexture("SkyAtmosphere ScatteringDensityLUT", Desc);

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
        FShaderInstance *TransmittanceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(TransmittanceShader->GetComputeShaderRHI());
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereTransmittanceShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetStorageImage("TransmittanceLUT", TransmittanceLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        RDGPassDesc PassDesc{"DispatchTransmittancePass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
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
        FShaderInstance *DirectIrradianceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(DirectIrradianceShader->GetComputeShaderRHI());
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereDirectIrradianceShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetStorageImage("IrradianceLUT", IrradianceLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetSampler("TransmittanceLUT", TransmittanceLUT->GetDefaultView());
        RDGPassDesc PassDesc{"DispatchDirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
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
        FShaderInstance *ScatteringShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(ScatteringShader->GetComputeShaderRHI());
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereScatteringShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetSampler("TransmittanceLUT", TransmittanceLUT->GetDefaultView());
        DescriptorSet->SetStorageImage("SingleScatteringRayleighLUT", DeltaScatteringRayleighLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetStorageImage("SingleScatteringMieLUT", SingleScatteringMieLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetStorageImage("MultiScatteringLUT", MultiScatteringLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        RDGPassDesc PassDesc{"DispatchScatteringPass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
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
        FShaderInstance *ScatteringDensityShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(ScatteringDensityShader->GetComputeShaderRHI(), { {EShaderStage::Compute, 0, 4} });
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereScatteringDensityShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetStorageImage("ScatteringDensityLUT", ScatteringDensityLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetSampler("TransmittanceLUT", TransmittanceLUT->GetDefaultView());
        DescriptorSet->SetSampler("SingleScatteringRayleighLUT", DeltaScatteringRayleighLUT->GetDefaultView());
        DescriptorSet->SetSampler("SingleScatteringMieLUT", SingleScatteringMieLUT->GetDefaultView());
        DescriptorSet->SetSampler("IrradianceLUT", IrradianceLUT->GetDefaultView());
        RDGPassDesc PassDesc{"DispatchScatteringDensityPass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
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
        FShaderInstance *IndirectIrradianceShader = GetGlobalShader(PermutationParameters);
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(IndirectIrradianceShader->GetComputeShaderRHI(), { {EShaderStage::Compute, 0, 4} });
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereIndirectIrradianceShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetStorageImage("IrradianceLUT", IrradianceLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetSampler("SingleScatteringRayleighLUT", DeltaScatteringRayleighLUT->GetDefaultView());
        DescriptorSet->SetSampler("SingleScatteringMieLUT", SingleScatteringMieLUT->GetDefaultView());
        RDGPassDesc PassDesc{"DispatchIndirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
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
        FShaderInstance *MultiScatteringShader = GetGlobalShader<FAtmosphereMultiScatteringShader>();
        RHIComputePipelineState *PSO = RHICreateComputePipelineState(MultiScatteringShader->GetComputeShaderRHI());
        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FAtmosphereMultiScatteringShader>(0, 0);
        DescriptorSet->SetUniformBuffer("AtmosphereParametersBlock", AtmosphereParameters);
        DescriptorSet->SetStorageImage("DeltaScatteringLUT", DeltaScatteringRayleighLUT->GetDefaultView(), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetStorageImage("MultiScatteringLUT", MultiScatteringLUT->GetDefaultView(), ERHIAccess::ShaderResourceReadWrite);
        DescriptorSet->SetSampler("TransmittanceLUT", TransmittanceLUT->GetDefaultView());
        DescriptorSet->SetSampler("ScatteringDensityLUT", ScatteringDensityLUT->GetDefaultView());
        RDGPassDesc PassDesc{"DispatchIndirectIrradiancePass"};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                RHICmdList.DispatchCompute(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
            });
    }

}