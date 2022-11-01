#include <waterbody/waterbody_constants.glsl>

#include "OpenGL/OpenGLUtils.h"
#include "DynamicRHI.h"
#include "Common/ShaderManager.h"
#include "Common/TextureManager.h"
#include "OceanScatteringPrecomputeSubPass.h"

//constexpr int WATER_TRANSMITTANCE_TEXTURE_WIDTH = 256;
//constexpr int WATER_TRANSMITTANCE_TEXTURE_HEIGHT = 64;
//
//constexpr int WATER_SCATTERING_TEXTURE_R_SIZE = 128;
//constexpr int WATER_SCATTERING_TEXTURE_MU_SIZE = 128;
//constexpr int WATER_SCATTERING_TEXTURE_MU_S_SIZE = 32;
//constexpr int WATER_SCATTERING_TEXTURE_NU_SIZE = 8;
//
//constexpr int WATER_SCATTERING_TEXTURE_WIDTH =
//WATER_SCATTERING_TEXTURE_NU_SIZE * WATER_SCATTERING_TEXTURE_MU_S_SIZE;
//constexpr int WATER_SCATTERING_TEXTURE_HEIGHT = WATER_SCATTERING_TEXTURE_MU_SIZE;
//constexpr int WATER_SCATTERING_TEXTURE_DEPTH = WATER_SCATTERING_TEXTURE_R_SIZE;
//
//constexpr int WATER_IRRADIANCE_TEXTURE_WIDTH = 64;
//constexpr int WATER_IRRADIANCE_TEXTURE_HEIGHT = 16;
//
//constexpr int NUM_SCATTERING_ORDERS = 4;

namespace und {
    void und::OceanScatteringPrecomputeSubPass::Initialize(std::shared_ptr<SceneObjectWaterbody> waterbody)
    {
        m_ScatteringLUT = _GM->RHICreateTexture3D("OceanScatteringLUT", EPixelFormat::PF_R32G32B32A32F, 1, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH, nullptr);
        m_MultiScatteringDensityLUT = _GM->RHICreateTexture3D("MultiScatteringDensityLUT", EPixelFormat::PF_R32G32B32A32F, 1, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH, nullptr);
        //OpenGLTextureImage3DRef SingleScatteringLUT = _GM->RHICreateTextureImage3D("OceanSingleScatteringLUT", GL_RGBA32F, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH);


        DrawOrCompute(waterbody);

        g_pTextureManager->AddGlobalTexture("OceanScatteringLUT", m_ScatteringLUT);
        g_pTextureManager->AddGlobalTexture("OceanScatteringDensityLUT", m_MultiScatteringDensityLUT);
        //g_pTextureManager->AddGlobalTexture("ScatteringDensityLUT", m_ScatteringDensityLUTs[1]);

    }
    void OceanScatteringPrecomputeSubPass::DrawOrCompute(std::shared_ptr<SceneObjectWaterbody> waterbody)
    {
        _GM->GLDEBUG();

        std::vector<RHITexture3DRef> m_ScatteringDensityLUTs;
        std::vector<RHITexture3DRef> m_MultiScatteringLUTs;
        m_ScatteringDensityLUTs.resize(NUM_SCATTERING_ORDERS);
        m_MultiScatteringLUTs.resize(NUM_SCATTERING_ORDERS);
        for (int i = 0; i < NUM_SCATTERING_ORDERS; i++)
        {
            m_MultiScatteringLUTs[i] = _GM->RHICreateTexture3D("", EPixelFormat::PF_R32G32B32A32F, 1, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH, nullptr);
        }
        for (int i = 0; i < NUM_SCATTERING_ORDERS; i++)
        {
            m_ScatteringDensityLUTs[i] = _GM->RHICreateTexture3D("", EPixelFormat::PF_R32G32B32A32F, 1, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH, nullptr);
        }
        _GM->GLDEBUG();

        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody SingleScattering Precompute"));
        _GM->GLDEBUG();
        _GM->RHIBindTexture(0, m_MultiScatteringLUTs[0], EDataAccessFlag::DA_WriteOnly);
        _GM->GLDEBUG();
        _GM->RHIBindTexture(1, m_ScatteringDensityLUTs[0], EDataAccessFlag::DA_WriteOnly);
        _GM->GLDEBUG();
        SetWaterbodyParameters(waterbody);
        _GM->GLDEBUG();
        _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);

        RHITextureParams params;
        params.Mag_Filter = ETextureFilters::TF_Linear;
        params.Min_Filter = ETextureFilters::TF_Linear;
        params.Wrap_S = ETextureWrapModes::TW_Clamp;
        params.Wrap_T = ETextureWrapModes::TW_Clamp;
        params.Wrap_R = ETextureWrapModes::TW_Clamp;
        _GM->GLDEBUG();
        for (int scattering_order = 2; scattering_order <= NUM_SCATTERING_ORDERS; scattering_order++)
        {
            int LUTs_index = scattering_order - 1;
            _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody ScatteringDensity Precompute"));
            SetWaterbodyParameters(waterbody);
            _GM->RHIBindTexture("ScatteringLUT_Input", m_MultiScatteringLUTs[LUTs_index-1], params);
            _GM->RHIBindTexture(0, m_ScatteringDensityLUTs[LUTs_index], EDataAccessFlag::DA_WriteOnly);
            _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);


            _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody MultiScattering Precompute"));
            SetWaterbodyParameters(waterbody);
            _GM->RHIBindTexture("ScatteringDensityLUT", m_ScatteringDensityLUTs[LUTs_index], params);
            _GM->RHIBindTexture(0, m_MultiScatteringLUTs[LUTs_index], EDataAccessFlag::DA_WriteOnly);
            _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);
        }
        params.Mag_Filter = ETextureFilters::TF_Nearest;
        params.Min_Filter = ETextureFilters::TF_Nearest;
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody Scattering Synthesize Precompute"));
        char uniformName[64];
        for (int i = 0; i < NUM_SCATTERING_ORDERS; i++)
        {
            sprintf(uniformName, "MultiScatteringLUTs[%d]", i);
            _GM->RHIBindTexture(uniformName, m_MultiScatteringLUTs[i], params);
            sprintf(uniformName, "ScatteringDensityLUTs[%d]", i);
            _GM->RHIBindTexture(uniformName, m_ScatteringDensityLUTs[i], params);
        }
        _GM->RHIBindTexture(0, m_ScatteringLUT, EDataAccessFlag::DA_WriteOnly);
        _GM->RHIBindTexture(1, m_MultiScatteringDensityLUT, EDataAccessFlag::DA_WriteOnly);
        _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);

    }
}
