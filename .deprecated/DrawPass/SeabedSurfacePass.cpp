#include "Common/SceneManager.h"
#include "Common/ShaderManager.h"
#include "Common/AssetLoader.h"
#include "Common/TextureManager.h"
#include "Interface/IApplication.h"
#include "DynamicRHI.h"
#include "OpenGL/OpenGLUtils.h"
#include "SeabedSurfacePass.h"
#include "QuadTreeSubPass.h"

constexpr int CAUSTIC_ANIMATION_FRAME_NUM = 64;
constexpr int CAUSTIC_ANIMATION_FRAMES_PER_SECOND = 16;
constexpr float CAUSTIC_ANIMATION_FRAME_TIME = 1.0f / CAUSTIC_ANIMATION_FRAMES_PER_SECOND;

namespace und {
    float last_caustic_frame_switch_time = 0;
    int caustic_frame_index = 0;
    int SeabedSurfacePass::Initialize(FrameVariables &frame)
    {
        glm::vec3 a = glm::refract(-glm::normalize(glm::vec3(1, 0, 0.90)), glm::vec3(0, 0, -1), 1.333f);
        float c = glm::degrees(glm::acos(glm::dot(a, glm::vec3(0, 0, 1))));
        HugeSurfaceInit(g_pSceneManager->GetScene()->SeabedSurface);

       g_pTextureManager->AddGlobalTexture("Seabed LODMap", m_QuadUpdatePass->LODMap, true);

        auto seabed_surface = std::dynamic_pointer_cast<SceneObjectTerrainSurface>(m_Surface);
        auto seabed_material = seabed_surface->TerrainMaterial;
        m_HeightMap = UploadTexture(seabed_surface->HeightMap, true);
        m_MaterialBaseColorMap = UploadTexture(seabed_material->GetBaseColor().ValueMap);
        _GM->RHIGenerateMipmap(m_MaterialBaseColorMap);
        m_MaterialNormalMap = UploadTexture(seabed_material->GetNormal().ValueMap, true);
        _GM->RHIGenerateMipmap(m_MaterialNormalMap);
        m_MaterialRoughnessMap = UploadTexture(seabed_material->GetRoughness().ValueMap, true);
        _GM->RHIGenerateMipmap(m_MaterialRoughnessMap);
        //std::shared_ptr<und::Image> img = seabed_surface->HeightMap->GetTextureImage();
        //int format = img->GetPixelFormat();
        //m_HeightMap = _GM->RHICreateTexture2D("SeabedHeightMap", format, img->Width, img->Height, img->data);
        m_Normal = _GM->RHICreateTexture2D("SeabedNormal", EPixelFormat::PF_R32G32B32A32F, 1, m_HeightMap->GetSizeX(), m_HeightMap->GetSizeY(), nullptr);

#ifdef _DEBUG
       g_pTextureManager->AddGlobalTexture("SeabedNormal", m_Normal);
#endif // _DEBUG
       RHITextureParams params;
       params.Mag_Filter = ETextureFilters::TF_Nearest;
       params.Mag_Filter = ETextureFilters::TF_Nearest;
       params.Wrap_T = ETextureWrapModes::TW_Clamp;
       params.Wrap_S = ETextureWrapModes::TW_Clamp;
       _GM->RHIUseShaderProgram(
           g_pShaderManager->GetShaderByName("Seabed Normal"));
       _GM->RHIBindTexture("DEM", m_HeightMap, params);
       _GM->RHIBindTexture(1, m_Normal, EDataAccessFlag::DA_WriteOnly);
       _GM->RHISetShaderParameter("DEMWidth", m_HeightMap->GetSizeX());
       _GM->RHISetShaderParameter("DEMHeight", m_HeightMap->GetSizeY());
       _GM->RHISetShaderParameter("PixelMeterSize",
                                  seabed_surface->PixelMeterSize);
       _GM->RHIDispatch(m_HeightMap->GetSizeX(), m_HeightMap->GetSizeY(), 1);

       char filename[32];
       for (int i = 0; i < CAUSTIC_ANIMATION_FRAME_NUM; i++) {
         sprintf(filename, "caustic\\caustic_512_%03d.bmp", i + 1);
         SceneObjectTexture tex(g_pAssetLoader->AssetDir + filename);
         auto img = tex.GetTextureImage();
         auto texture =
             _GM->RHICreateTexture2D("", img->GetPixelFormat(), 1, img->Width,
                                     img->Height, img->data);
         m_Caustics.push_back(texture);
        }
#ifdef _DEBUG
       g_pTextureManager->AddGlobalTexture("Caustics", m_Caustics[0]);
#endif // _DEBUG
        //m_ScatteringLUT = _GM->RHICreateTextureImage3D("OceanScatteringLUT", EPixelFormat::PF_R32G32B32A32F, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH);
        ////OpenGLTextureImage3DRef SingleScatteringLUT = _GM->RHICreateTextureImage3D("OceanSingleScatteringLUT", EPixelFormat::PF_R32G32B32A32F, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH);

        //OpenGLTextureImage3DRef ScatteringDensityLUTs[NUM_SCATTERING_ORDERS];
        //OpenGLTextureImage3DRef MultiScatteringLUTs[NUM_SCATTERING_ORDERS];
        //for (int i = 0; i < NUM_SCATTERING_ORDERS; i++)
        //{
        //    MultiScatteringLUTs[i] = _GM->RHICreateTextureImage3D("", EPixelFormat::PF_R32G32B32A32F, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH);
        //}
        //for (int i = 1; i < NUM_SCATTERING_ORDERS; i++)
        //{
        //    ScatteringDensityLUTs[i] = _GM->RHICreateTextureImage3D("", EPixelFormat::PF_R32G32B32A32F, WATER_SCATTERING_TEXTURE_WIDTH, WATER_SCATTERING_TEXTURE_HEIGHT, WATER_SCATTERING_TEXTURE_DEPTH);
        //}

        //_GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody SingleScattering Precompute")); 
        //_GM->RHIBindTexture(0, MultiScatteringLUTs[0], EDataAccessFlag::DA_WriteOnly);
        //SetWaterbodyParameters(g_pSceneManager->GetScene()->Waterbody);
        //_GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);

        //OpenGLTextureParams params;
        //params.Mag_Filter = ETextureFilters::TF_Linear;
        //params.Min_Filter = ETextureFilters::TF_Linear;
        //params.Wrap_S = ETextureWrapModes::TW_Clamp;
        //params.Wrap_T = ETextureWrapModes::TW_Clamp;
        //params.Wrap_R = ETextureWrapModes::TW_Clamp;
        //for (int scattering_order = 2; scattering_order <= NUM_SCATTERING_ORDERS; scattering_order++)
        //{
        //    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody ScatteringDensity Precompute"));
        //    SetWaterbodyParameters(g_pSceneManager->GetScene()->Waterbody);
        //    _GM->RHIBindTexture("ScatteringLUT_Input", MultiScatteringLUTs[scattering_order - 2], params);
        //    _GM->RHIBindTexture(0, ScatteringDensityLUTs[scattering_order - 1], EDataAccessFlag::DA_WriteOnly);
        //    _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);


        //    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody MultiScattering Precompute"));
        //    SetWaterbodyParameters(g_pSceneManager->GetScene()->Waterbody);
        //    _GM->RHIBindTexture("ScatteringDensityLUT", ScatteringDensityLUTs[scattering_order-1], params);
        //    _GM->RHIBindTexture(0, MultiScatteringLUTs[scattering_order-1], EDataAccessFlag::DA_WriteOnly);
        //    _GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);
        //}
        //params.Mag_Filter = ETextureFilters::TF_Nearest;
        //params.Min_Filter = ETextureFilters::TF_Nearest;
        //_GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Waterbody Scattering Synthesize Precompute"));
        //char uniformName[64];
        //for (int i = 0; i < NUM_SCATTERING_ORDERS; i++)
        //{
        //    sprintf(uniformName, "MultiScatteringLUTs[%d]", i);
        //    _GM->RHIBindTexture(uniformName, MultiScatteringLUTs[i], params);
        //}
        //_GM->RHIBindTexture(0, m_ScatteringLUT, EDataAccessFlag::DA_WriteOnly);
        //_GM->RHIDispatch(WATER_SCATTERING_TEXTURE_WIDTH / 8, WATER_SCATTERING_TEXTURE_HEIGHT / 8, WATER_SCATTERING_TEXTURE_DEPTH / 8);

        //_GM->TexMngr.AddGlobalTexture("OceanScatteringLUT", m_ScatteringLUT);
        //_GM->TexMngr.AddGlobalTexture("ScatteringDensityLUT", ScatteringDensityLUTs[1]);
        return 0;
    }

    void SeabedSurfacePass::Draw(FrameVariables &frame)
    {
        auto seabed_surface = std::dynamic_pointer_cast<SceneObjectTerrainSurface>(m_Surface);
        HugeSurfaceUpdatePatchList(frame, m_HeightMap, nullptr, seabed_surface->HeightMapMeterSize, ETextureWrapModes::TW_Clamp);

#ifdef _DEBUG
        char uniformName[256];
        for (int LOD = 0; LOD < m_QuadUpdatePass->MinMaxMap.size(); LOD++)
        {
            sprintf(uniformName, "Seabed MinMaxMap[%d]", LOD);
           g_pTextureManager->AddGlobalTexture(uniformName, m_QuadUpdatePass->MinMaxMap[LOD], true);
        }
#endif // _DEBUG
        //glm::ortho()
        //auto seabed_surface = std::dynamic_pointer_cast<SceneObjectTerrainSurface>(m_Surface);
        //auto seabed_material = seabed_surface->TerrainMaterial;
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Seabed Shading"));
        _GM->RHISetShaderParameter("TextureMapMeterSize", seabed_surface->MaterialMeterSize);
        float time = g_pApp->GetTimeSinceStart();
        int caustic_frame_index = time / CAUSTIC_ANIMATION_FRAME_TIME;
        caustic_frame_index = caustic_frame_index % CAUSTIC_ANIMATION_FRAME_NUM;
        _GM->RHIBindTexture("causticMap", m_Caustics[caustic_frame_index]);
        _GM->RHISetShaderParameter("Time", time);
        auto ocean_surface = g_pSceneManager->GetScene()->OceanSurface;
        _GM->RHISetShaderParameter("WindDirection", glm::normalize(ocean_surface->WindDirection));
        SetWaterbodyParameters(g_pSceneManager->GetScene()->Waterbody);

        RHITextureParams params;
        params.Mag_Filter = ETextureFilters::TF_Linear;
        params.Min_Filter = ETextureFilters::TF_Linear_Mipmap_Linear;
        _GM->RHIBindTexture("baseColorMap", m_MaterialBaseColorMap, params);
        _GM->RHIBindTexture("normalMap", m_MaterialNormalMap, params);
        _GM->RHIBindTexture("roughnessMap", m_MaterialRoughnessMap, params);

        params.Mag_Filter = ETextureFilters::TF_Linear;
        params.Min_Filter = ETextureFilters::TF_Linear;
        params.Wrap_S = ETextureWrapModes::TW_Clamp;
        params.Wrap_T = ETextureWrapModes::TW_Clamp;
        _GM->RHIBindTexture("HeightMap", m_HeightMap, params);
        _GM->RHIBindTexture("BaseNormal", m_Normal, params);
#if defined(_DEBUG) && defined(DIRECT_DISPATCH)
        std::cout << "Seabed patch num: " << m_PatchNumToDraw << std::endl;
#endif // _DEBUG

        HugeSurfacePass::Draw(frame);





    }

}
