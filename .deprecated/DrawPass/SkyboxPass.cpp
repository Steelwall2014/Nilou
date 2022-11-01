#include "SkyboxPass.h"
#include "Common/ShaderManager.h"
#include "Common/SceneManager.h"
#include "Common/TextureManager.h"
#include "RHIDefinitions.h"
#include "DynamicRHI.h"
#include "OpenGL/OpenGLUtils.h"

#include <atmosphere/atmosphere_constants.glsl>


constexpr float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

glm::vec3 last_lightDirection = glm::vec3(0, 0, 0);
void und::SkyboxPass::UpdateSkyMap(FrameVariables &frame)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere SkyMap"));
    _GM->RHIBindTexture(0, m_SkyMap, EDataAccessFlag::DA_WriteOnly);
    RHITextureParams params;
    params.Mag_Filter = ETextureFilters::TF_Linear;
    params.Min_Filter = ETextureFilters::TF_Linear;
    params.Wrap_S = ETextureWrapModes::TW_Clamp;
    params.Wrap_T = ETextureWrapModes::TW_Clamp;
    params.Wrap_R = ETextureWrapModes::TW_Clamp;
    _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT, params);
    _GM->RHIBindTexture("SingleScatteringRayleighLUT", m_MultiScatteringLUT, params);
    _GM->RHIBindTexture("SingleScatteringMieLUT", m_SingleScatteringMieLUT, params);
    _GM->RHISetShaderParameter("SunLightDir", frame.frameContext.lights[0].lightDirection);
    _GM->RHISetShaderParameter("SkyMapSize", glm::vec2(1024, 1024));
    SetAtmosphereParameters(m_Atmosphere);
    _GM->RHIDispatch(32, 32, 1);
}
int und::SkyboxPass::Initialize(FrameVariables &frame)
{
    _GM->GLDEBUG();
    m_Atmosphere = g_pSceneManager->GetScene()->Atmosphere;
    m_TransmittanceLUT = _GM->RHICreateTexture2D("TransmittanceLUT", EPixelFormat::PF_R32G32B32A32F, 1, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, nullptr);
    m_IrradianceLUT = _GM->RHICreateTexture2D("IrradianceLUT", EPixelFormat::PF_R32G32B32A32F, 1, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, nullptr);
    m_DeltaScatteringRayleighLUT = _GM->RHICreateTexture3D("SingleScatteringRayleighLUT", EPixelFormat::PF_R32G32B32A32F, 1, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);
    m_SingleScatteringMieLUT = _GM->RHICreateTexture3D("SingleScatteringMieLUT", EPixelFormat::PF_R32G32B32A32F, 1, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);
    m_MultiScatteringLUT = _GM->RHICreateTexture3D("MultiScatteringLUT", EPixelFormat::PF_R32G32B32A32F, 1, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);
    //OpenGLTexture3DRef MultiScatteringLUT_TempA = _GM->RHICreateTextureImage3D("MultiScatteringLUT_TempA", EPixelFormat::PF_R32G32B32A32F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH);
    //OpenGLTexture3DRef MultiScatteringLUT_TempB = _GM->RHICreateTextureImage3D("MultiScatteringLUT_TempB", EPixelFormat::PF_R32G32B32A32F, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH);
    RHITexture3DRef ScatteringDensityLUT = _GM->RHICreateTexture3D("SingleScatteringMieLUT", EPixelFormat::PF_R32G32B32A32F, 1, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr);

    _GM->GLDEBUG();
    g_pTextureManager->AddGlobalTexture("TransmittanceLUT", m_TransmittanceLUT);
    g_pTextureManager->AddGlobalTexture("IrradianceLUT", m_IrradianceLUT);
    g_pTextureManager->AddGlobalTexture("SingleScatteringRayleighLUT", m_MultiScatteringLUT);
    g_pTextureManager->AddGlobalTexture("SingleScatteringMieLUT", m_SingleScatteringMieLUT);
    g_pTextureManager->AddGlobalTexture("SkyScatteringDensityLUT", ScatteringDensityLUT);
    
    _GM->GLDEBUG();
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere Transmittance Precompute"));
    _GM->RHIBindTexture(0, m_TransmittanceLUT, EDataAccessFlag::DA_WriteOnly);
    SetAtmosphereParameters(m_Atmosphere);
    _GM->RHIDispatch(TRANSMITTANCE_TEXTURE_WIDTH / 8, TRANSMITTANCE_TEXTURE_HEIGHT / 8, 1);

    _GM->GLDEBUG();
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere Direct Irradiance Precompute"));
    _GM->RHIBindTexture(0, m_IrradianceLUT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT);
    SetAtmosphereParameters(m_Atmosphere);
    _GM->RHIDispatch(IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);

    _GM->GLDEBUG();
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere Scattering Precompute"));
    _GM->RHIBindTexture(0, m_DeltaScatteringRayleighLUT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture(1, m_SingleScatteringMieLUT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture(2, m_MultiScatteringLUT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT);
    SetAtmosphereParameters(m_Atmosphere);
    _GM->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);

    _GM->GLDEBUG();
    int num_scattering_orders = 4;
    for (int scattering_order = 2;
        scattering_order <= num_scattering_orders;
        ++scattering_order) {

    _GM->GLDEBUG();
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere Scattering Density Precompute"));
        _GM->RHIBindTexture(0, ScatteringDensityLUT, EDataAccessFlag::DA_WriteOnly);
        _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT);
        _GM->RHIBindTexture("SingleScatteringRayleighLUT", m_DeltaScatteringRayleighLUT);
        _GM->RHIBindTexture("SingleScatteringMieLUT", m_SingleScatteringMieLUT);
        _GM->RHIBindTexture("IrradianceLUT", m_IrradianceLUT);
        _GM->RHISetShaderParameter("scattering_order", scattering_order);
        SetAtmosphereParameters(m_Atmosphere);
        _GM->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);

    _GM->GLDEBUG();
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere Indirect Irradiance Precompute"));
        _GM->RHIBindTexture(0, m_IrradianceLUT, EDataAccessFlag::DA_WriteOnly);
        _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT);
        _GM->RHIBindTexture("SingleScatteringRayleighLUT", m_DeltaScatteringRayleighLUT);
        _GM->RHIBindTexture("SingleScatteringMieLUT", m_SingleScatteringMieLUT);
        _GM->RHISetShaderParameter("scattering_order", scattering_order);
        SetAtmosphereParameters(m_Atmosphere);
        _GM->RHIDispatch(IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);

    _GM->GLDEBUG();
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Atmosphere MultiScattering Precompute"));
        _GM->RHIBindTexture(0, m_DeltaScatteringRayleighLUT, EDataAccessFlag::DA_WriteOnly);
        _GM->RHIBindTexture(1, m_MultiScatteringLUT, EDataAccessFlag::DA_ReadWrite);
        _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT);
        _GM->RHIBindTexture("ScatteringDensityLUT", ScatteringDensityLUT);
        SetAtmosphereParameters(m_Atmosphere);
        _GM->RHIDispatch(SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
    }
    _GM->GLDEBUG();
    m_SkyMap = _GM->RHICreateTexture2D("SkyMap", EPixelFormat::PF_R32G32B32A32F, 1, 1024, 1024, nullptr);
    g_pTextureManager->AddGlobalTexture("SkyMap", m_SkyMap);

    _GM->GLDEBUG();
    m_SkyboxVAO = _GM->RHICreateVertexArrayObject(EPrimitiveMode::PM_Triangles);
    auto vbo = _GM->RHICreateVertexAttribBuffer(
        0, sizeof(skyboxVertices), (void *)skyboxVertices, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
    m_SkyboxVAO->AddVertexAttribBuffer(vbo.first, vbo.second);
    _GM->RHIInitializeVertexArrayObject(m_SkyboxVAO);

    _GM->GLDEBUG();
    void *data[6];
    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        auto texture = g_pSceneManager->GetScene()->Skybox->GetTexture(i);
        auto image = texture->GetTextureImage();
        width = image->Width;
        height = image->Height;
        data[i] = image->data;
    }
    _GM->GLDEBUG();
    auto SkyboxTexture = _GM->RHICreateTextureCube(
        "SkyboxTexture", EPixelFormat::PF_R8G8B8, 1, width, height, data);
    _GM->GLDEBUG();
    g_pTextureManager->AddGlobalTexture("SkyboxTexture", SkyboxTexture);
    _GM->GLDEBUG();
	return 0;
}

void und::SkyboxPass::Draw(FrameVariables &frame)
{
    if (frame.frameContext.lights[0].lightDirection != last_lightDirection)
    {
        UpdateSkyMap(frame);
        last_lightDirection = frame.frameContext.lights[0].lightDirection;
    }

    _GM->GLDEBUG();
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Skybox"));

    RHITextureParams params;
    params.Mag_Filter = ETextureFilters::TF_Linear;
    params.Min_Filter = ETextureFilters::TF_Linear;
    params.Wrap_S = ETextureWrapModes::TW_Clamp;
    params.Wrap_T = ETextureWrapModes::TW_Clamp;
    params.Wrap_R = ETextureWrapModes::TW_Clamp;
    _GM->GLDEBUG();
    _GM->RHIBindTexture("TransmittanceLUT", m_TransmittanceLUT, params);
    _GM->RHIBindTexture("SingleScatteringRayleighLUT", m_MultiScatteringLUT, params);
    _GM->RHIBindTexture("SingleScatteringMieLUT", m_SingleScatteringMieLUT, params);
    _GM->RHISetShaderParameter("SunLightDir", frame.frameContext.lights[0].lightDirection);
    _GM->RHISetShaderParameter("cameraPos", frame.frameContext.cameraPosition);
    SetAtmosphereParameters(m_Atmosphere);

    _GM->GLDEBUG();
    und::RHIDepthStencilState state;
    state.DepthFunc = ECompareFunction::CF_LessEqual;
    _GM->RHISetDepthStencilState(state);
    glm::mat4 view = glm::mat4(glm::mat3(frame.frameContext.viewMatrix));

    //auto m_SkyboxTexture = g_pTextureManager->GetGlobalTexture("SkyboxTexture");
    //OpenGLTextureParams params;
    //params.Wrap_S = ETextureWrapModes::TW_Clamp;
    //params.Wrap_T = ETextureWrapModes::TW_Clamp;
    //params.Wrap_R = ETextureWrapModes::TW_Clamp;
    //_GM->RHIBindTexture("skybox", m_SkyboxTexture, params);
    _GM->GLDEBUG();
    _GM->RHISetShaderParameter("VP", frame.frameContext.projectionMatrix * view);
    _GM->RHIDrawVertexArray(m_SkyboxVAO, frame.frameContext.renderTarget);
    state.DepthFunc = ECompareFunction::CF_Less;
    _GM->RHISetDepthStencilState(state);
    _GM->GLDEBUG();
}
