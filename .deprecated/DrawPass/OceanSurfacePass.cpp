#include "DynamicRHI.h"
#include "OpenGL/OpenGLUtils.h"
#include "Common/SceneManager.h"
#include "Common/ShaderManager.h"
#include "Common/TextureManager.h"
#include "Common/AssetLoader.h"
#include "Interface/IApplication.h"
#include "OceanSurfacePass.h"
#include "QuadTreeSubPass.h"

#ifdef _DEBUG
#include "Common/InputManager.h"
#endif // _DEBUG

glm::vec2 sigmaSq;

void und::OceanSurfacePass::Occean_ComputeGaussionSpectrums(unsigned int N, int group_x, int group_y)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean Gaussion"));
    _GM->RHISetShaderParameter("N", N);
    _GM->RHIBindTexture(0, GaussianRandomRT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIDispatch(group_x, group_y, 1);
    _GM->RHIImageMemoryBarrier();
    _GM->GLDEBUG();
}

void und::OceanSurfacePass::Ocean_ComputeDisplacementSpectrums(unsigned int N, glm::vec2 WindDirection, float WindSpeed, float TimeToNow, float A, float Lxy, int group_x, int group_y)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean Spectrums"));
    _GM->RHISetShaderParameter("N", N);
    _GM->RHISetShaderParameter("WindDirection", WindDirection);
    _GM->RHISetShaderParameter("WindSpeed", WindSpeed);
    _GM->RHISetShaderParameter("Lxy", Lxy);

    // 这里1000.0f是一个magic number，如果去掉的话每个海面tile中心会有一个莫名其妙的同心圆波浪，这个波浪随着时间扩散，
    // 为了去掉这个波浪人为把时间加一个数，这样这个波浪就显现不出来了
#ifdef _DEBUG
    if (!PauseOceanSurface)
        _GM->RHISetShaderParameter("Time", TimeToNow + 1000.0f);
#else
    _GM->RHISetShaderParameter("Time", TimeToNow + 1000.0f);
#endif // _DEBUG


    _GM->RHISetShaderParameter("A", A);
    _GM->RHIBindTexture(0, GaussianRandomRT, EDataAccessFlag::DA_ReadOnly);
    _GM->RHIBindTexture(1, HeightSpectrumRT, EDataAccessFlag::DA_ReadWrite);
    _GM->RHIBindTexture(2, DisplaceXSpectrumRT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture(3, DisplaceYSpectrumRT, EDataAccessFlag::DA_WriteOnly);

    _GM->RHIDispatch(group_x, group_y, 1);
    _GM->RHIImageMemoryBarrier();
    _GM->GLDEBUG();
}

void und::OceanSurfacePass::Ocean_ComputeFFT(und::RHITexture2DRef &input, unsigned int N, unsigned int Ns, int function, int group_x, int group_y)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean FFT"));

    _GM->RHISetShaderParameter("N", N);
    _GM->RHISetShaderParameter("Ns", Ns);
    _GM->RHISetShaderParameter("FUNCTION", function);
    _GM->RHIBindTexture(0, input, EDataAccessFlag::DA_ReadOnly);
    _GM->RHIBindTexture(1, OutputRT, EDataAccessFlag::DA_WriteOnly);

    _GM->RHIDispatch(group_x, group_y, 1);
    _GM->RHIImageMemoryBarrier();

    std::swap(input, OutputRT);
    //und::OpenGLTextureImage2DRef temp = input;
    //input = OutputRT;
    //OutputRT = temp;
    _GM->GLDEBUG();
}

void und::OceanSurfacePass::Ocean_ComputeDisplacement(unsigned int N, float Scale, int group_x, int group_y)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean Displacement"));
    _GM->RHISetShaderParameter("N", N);
    _GM->RHISetShaderParameter("Scale", Scale);
    
    _GM->RHIBindTexture(0, HeightSpectrumRT, EDataAccessFlag::DA_ReadWrite);
    _GM->RHIBindTexture(1, DisplaceXSpectrumRT, EDataAccessFlag::DA_ReadWrite);
    _GM->RHIBindTexture(2, DisplaceYSpectrumRT, EDataAccessFlag::DA_ReadWrite);
    _GM->RHIBindTexture(3, DisplaceRT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIDispatch(group_x, group_y, 1);
    _GM->RHIImageMemoryBarrier();
    _GM->GLDEBUG();
}

void und::OceanSurfacePass::Ocean_ComputeNormalFoam(unsigned int N, float FFTPixelMeterSize, int group_x, int group_y)
{
    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean Normal and Foam"));
    _GM->RHISetShaderParameter("N", N);
    _GM->RHISetShaderParameter("FFTPixelMeterSize", FFTPixelMeterSize);

    _GM->RHIBindTexture(0, DisplaceRT, EDataAccessFlag::DA_ReadOnly);
    _GM->RHIBindTexture(1, NormalRT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIBindTexture(2, FoamRT, EDataAccessFlag::DA_WriteOnly);
    _GM->RHIDispatch(group_x, group_y, 1);
    _GM->RHIImageMemoryBarrier();
    _GM->GLDEBUG();
}

void und::OceanSurfacePass::Ocean_UpdateOceanSurfaceTextures(float time)
{
    auto ocean_surface = std::dynamic_pointer_cast<SceneObjectOceanSurface>(m_Surface);
    int             FFTPow = ocean_surface->FFTPow;
    unsigned int    N = pow(2, FFTPow);
    glm::vec2       WindDirection = ocean_surface->WindDirection;
    float           WindSpeed = ocean_surface->WindSpeed;
    int             group_x = N / 32;
    int             group_y = N / 32;     // 这里的32是写死在glsl中的"local_size"
    float           A = ocean_surface->A;
    float           FFTPixelMeterSize = ocean_surface->FFTPixelMeterSize;
    float           Lxy = FFTPixelMeterSize * N;
    float           Scale = 1;

    Ocean_ComputeDisplacementSpectrums(N, WindDirection, WindSpeed, time, A, Lxy, group_x, group_y);

    for (int m = 1; m <= FFTPow; m++)
    {
        unsigned int ns = pow(2, m - 1);
        Ocean_ComputeFFT(HeightSpectrumRT, N, ns, 0, group_x, group_y);
        Ocean_ComputeFFT(DisplaceXSpectrumRT, N, ns, 0, group_x, group_y);
        Ocean_ComputeFFT(DisplaceYSpectrumRT, N, ns, 0, group_x, group_y);

    }
    for (int m = 1; m <= FFTPow; m++)
    {
        int ns = (int)pow(2, m - 1);
        Ocean_ComputeFFT(HeightSpectrumRT, N, ns, 2, group_x, group_y);
        Ocean_ComputeFFT(DisplaceXSpectrumRT, N, ns, 2, group_x, group_y);
        Ocean_ComputeFFT(DisplaceYSpectrumRT, N, ns, 2, group_x, group_y);
        
    }
    Ocean_ComputeDisplacement(N, Scale, group_x, group_y);
    Ocean_ComputeNormalFoam(N, FFTPixelMeterSize, group_x, group_y);
    g_pTextureManager->AddGlobalTexture("HeightSpectrumRT", HeightSpectrumRT, true);
}

int und::OceanSurfacePass::Initialize(FrameVariables &frame)
{
    // TODO 重做texture
    std::shared_ptr<SceneObjectTexture> PerlinNoiseTexture = std::make_shared<SceneObjectTexture>(AssetLoader::AssetDir + "perlin_noise.dds");
    PerlinNoise = UploadTexture(PerlinNoiseTexture, true);
    _GM->RHIGenerateMipmap(PerlinNoise);

    _GM->GLDEBUG();
    HugeSurfaceInit(g_pSceneManager->GetScene()->OceanSurface);

    g_pTextureManager->AddGlobalTexture("OceanSurface LODMap", m_QuadUpdatePass->LODMap, true);

#ifdef _DEBUG
    InputActionMapping pause_ocean("PauseOceanSurface");
    pause_ocean.AddGroup(InputKey::KEY_SPACE);
    g_pInputManager->BindAction(pause_ocean, InputEvent::IE_Pressed, this, &OceanSurfacePass::SwitchPauseOceanSurface);
#endif
    int             FFTPow = g_pSceneManager->GetScene()->OceanSurface->FFTPow;
    unsigned int    N = (unsigned int)pow(2, FFTPow);
    int             group_x = N / 32, group_y = N / 32;     // 这里的32是写死在glsl中的"local_size"
    int             LODMapSize = g_pSceneManager->GetScene()->OceanSurface->QTree->LODMapSize;
    _GM->GLDEBUG();
    GaussianRandomRT = _GM->RHICreateTexture2D("GaussianRandomRT", EPixelFormat::PF_R32G32B32A32F, 1, N, N, nullptr);
    HeightSpectrumRT = _GM->RHICreateTexture2D("HeightSpectrumRT", EPixelFormat::PF_R32G32B32A32F, 1, N, N, nullptr);
    DisplaceXSpectrumRT = _GM->RHICreateTexture2D("DisplaceXSpectrumRT", EPixelFormat::PF_R32G32B32A32F, 1, N, N, nullptr);
    DisplaceYSpectrumRT = _GM->RHICreateTexture2D("DisplaceYSpectrumRT", EPixelFormat::PF_R32G32B32A32F, 1, N, N, nullptr);
    DisplaceRT = _GM->RHICreateTexture2D("DisplaceRT", EPixelFormat::PF_R32G32B32A32F, FFTPow, N, N, nullptr);
    OutputRT = _GM->RHICreateTexture2D("OutputRT", EPixelFormat::PF_R32G32B32A32F, 1, N, N, nullptr);
    NormalRT = _GM->RHICreateTexture2D("NormalRT", EPixelFormat::PF_R32G32B32A32F, FFTPow, N, N, nullptr);
    FoamRT = _GM->RHICreateTexture2D("FoamRT", EPixelFormat::PF_R32G32B32A32F, FFTPow, N, N, nullptr);
    _GM->GLDEBUG();

#ifdef _DEBUG
    g_pTextureManager->AddGlobalTexture("PerlinNoise", PerlinNoise);
    g_pTextureManager->AddGlobalTexture("GaussianRandomRT", GaussianRandomRT);
    g_pTextureManager->AddGlobalTexture("HeightSpectrumRT", HeightSpectrumRT);
    g_pTextureManager->AddGlobalTexture("DisplaceXSpectrumRT", DisplaceXSpectrumRT);
    g_pTextureManager->AddGlobalTexture("DisplaceYSpectrumRT", DisplaceYSpectrumRT);
    g_pTextureManager->AddGlobalTexture("DisplaceRT", DisplaceRT);
    g_pTextureManager->AddGlobalTexture("NormalRT", NormalRT);
    g_pTextureManager->AddGlobalTexture("FoamRT", FoamRT);
#endif // _DEBUG

    Occean_ComputeGaussionSpectrums(N, group_x, group_y);

    //Ocean_UpdateOceanSurfaceTextures(0);
//    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Calculate Slope Sum"));
//    unsigned int size = N / 2;
//    OpenGLTextureImage2DRef TempSums_A = NormalRT;
//    OpenGLTextureImage2DRef TempSums_B;
//    for (int i = 0; i < FFTPow-1; i++)
//    {
//        TempSums_B = _GM->RHICreateTextureImage2D("", EPixelFormat::PF_R32G32B32A32F, size, size);
//        _GM->RHIBindTexture(0, TempSums_A, EDataAccessFlag::DA_ReadOnly);
//        _GM->RHIBindTexture(1, TempSums_B, EDataAccessFlag::DA_WriteOnly);
//        _GM->RHIDispatch(size, size, 1);
//        TempSums_A = TempSums_B;
//        size = size / 2;
//    }
//#ifdef _DEBUG
//    g_pTextureManager->AddGlobalTexture("Slope Sum", TempSums_B);
//#endif // _DEBUG
//
//    TempSums_A = _GM->RHICreateTextureImage2D("", EPixelFormat::PF_R32G32B32A32F, N, N);
//    _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Calculate Slope Variance"));
//    _GM->RHISetShaderParameter("N", N);
//    _GM->RHISetShaderParameter("IsInit", true);
//    _GM->RHIBindTexture(0, NormalRT, EDataAccessFlag::DA_ReadOnly);
//    _GM->RHIBindTexture(1, TempSums_A, EDataAccessFlag::DA_WriteOnly);
//    _GM->RHIBindTexture(2, TempSums_B, EDataAccessFlag::DA_ReadOnly);
//    _GM->RHIDispatch(N, N, 1);
//
//    size = N / 2;
//    for (int i = 0; i < FFTPow - 1; i++)
//    {
//        _GM->RHISetShaderParameter("IsInit", false);
//        TempSums_B = _GM->RHICreateTextureImage2D("", EPixelFormat::PF_R32G32B32A32F, size, size);
//        _GM->RHIBindTexture(0, TempSums_A, EDataAccessFlag::DA_ReadOnly);
//        _GM->RHIBindTexture(1, TempSums_B, EDataAccessFlag::DA_WriteOnly);
//        _GM->RHIDispatch(size, size, 1);
//        TempSums_A = TempSums_B;
//        size = size / 2;
//    }
//    SlopeVariance = TempSums_B;
//    glm::vec4 *pixels = (glm::vec4 *)_GM->RHIReadImagePixel(SlopeVariance);
//    sigmaSq = glm::vec2(
//        pixels[0].x + pixels[1].x + pixels[2].x + pixels[3].x,
//        pixels[0].y + pixels[1].y + pixels[2].y + pixels[3].y);
//    sigmaSq = sigmaSq / (float)N / (float)N;
//    delete[] pixels;
//#ifdef _DEBUG
//    g_pTextureManager->AddGlobalTexture("Slope Variance", TempSums_B);
//#endif // _DEBUG
	return 0;
}

void und::OceanSurfacePass::Draw(FrameVariables &frame)
{
    Ocean_UpdateOceanSurfaceTextures(g_pApp->GetTimeSinceStart());
    //Ocean_UpdateOceanSurfaceTextures(0);

    auto ocean_surface = std::dynamic_pointer_cast<SceneObjectOceanSurface>(m_Surface);
    HugeSurfaceUpdatePatchList(frame, HeightSpectrumRT, DisplaceRT, ocean_surface->HeightMapMeterSize, ETextureWrapModes::TW_Repeat);


#ifdef _DEBUG
    char uniformName[256];
    for (int LOD = 0; LOD < m_QuadUpdatePass->MinMaxMap.size(); LOD++)
    {
        sprintf(uniformName, "OceanSurface MinMaxMap[%d]", LOD);
        g_pTextureManager->AddGlobalTexture(uniformName, m_QuadUpdatePass->MinMaxMap[LOD], true);
    }
#endif // _DEBUG


    /*********海面纹理的更新*********/

    _GM->GLDEBUG();
	GDynamicRHI->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Ocean Surface Shading"));
    auto atmosphere = g_pSceneManager->GetScene()->Atmosphere;
    SetAtmosphereParameters(atmosphere);
    auto waterbody = g_pSceneManager->GetScene()->Waterbody;
    SetWaterbodyParameters(waterbody);
    //_GM->RHIBindTexture(0, SlopeVariance, EDataAccessFlag::DA_ReadOnly);
    RHITextureParams params;
    params.Mag_Filter = ETextureFilters::TF_Linear;
    params.Min_Filter = ETextureFilters::TF_Linear_Mipmap_Linear;
    _GM->GLDEBUG();
    _GM->RHIGenerateMipmap(DisplaceRT);
    _GM->GLDEBUG();
    _GM->RHIBindTexture("DisplaceMap", DisplaceRT, params);
    _GM->GLDEBUG();
    _GM->RHIGenerateMipmap(NormalRT);
    _GM->GLDEBUG();
    _GM->RHIBindTexture("NormalMap", NormalRT, params);
    _GM->GLDEBUG();
    _GM->RHIGenerateMipmap(FoamRT);
    _GM->GLDEBUG();
    _GM->RHIBindTexture("FoamMap", FoamRT, params);
    _GM->GLDEBUG();
    _GM->RHIBindTexture("PerlinNoise", PerlinNoise, params);
    _GM->GLDEBUG();
    params.Mag_Filter = ETextureFilters::TF_Linear;
    params.Min_Filter = ETextureFilters::TF_Linear;
    params.Wrap_S = ETextureWrapModes::TW_Clamp;
    params.Wrap_T = ETextureWrapModes::TW_Clamp;
    _GM->GLDEBUG();
    _GM->RHIBindTexture("skybox", g_pTextureManager->GetGlobalTexture("SkyboxTexture"), params);
    _GM->GLDEBUG();
    _GM->RHIBindTexture("skymap", g_pTextureManager->GetGlobalTexture("SkyMap"), params);
    _GM->GLDEBUG();
    RHITextureRef TransmittanceLUT = g_pTextureManager->GetGlobalTexture("TransmittanceLUT");
    _GM->GLDEBUG();
    _GM->RHIBindTexture("TransmittanceLUT", TransmittanceLUT, params);
    _GM->GLDEBUG();
    RHITextureRef SingleScatteringRayleighLUT = g_pTextureManager->GetGlobalTexture("SingleScatteringRayleighLUT");
    _GM->GLDEBUG();
    _GM->RHIBindTexture("SingleScatteringRayleighLUT", SingleScatteringRayleighLUT, params);
    _GM->GLDEBUG();
    RHITextureRef SingleScatteringMieLUT = g_pTextureManager->GetGlobalTexture("SingleScatteringMieLUT");
    _GM->GLDEBUG();
    _GM->RHIBindTexture("SingleScatteringMieLUT", SingleScatteringMieLUT, params);
    _GM->GLDEBUG();
    RHITextureRef OceanScatteringLUT = g_pTextureManager->GetGlobalTexture("OceanScatteringLUT");
    _GM->GLDEBUG();
    _GM->RHIBindTexture("OceanScatteringLUT", OceanScatteringLUT, params);
    _GM->GLDEBUG();

    _GM->RHISetShaderParameter("OceanTextureMeterSize", ocean_surface->HeightMapMeterSize);
    _GM->GLDEBUG();
    _GM->RHISetShaderParameter("A", ocean_surface->A);
    _GM->GLDEBUG();
    _GM->RHISetShaderParameter("WindDirection", ocean_surface->WindDirection);
    _GM->GLDEBUG();
#ifdef _DEBUG
    if (!PauseOceanSurface)
        _GM->RHISetShaderParameter("Time", g_pApp->GetTimeSinceStart());
#endif // _DEBUG

    
    _GM->GLDEBUG();
    RHIRasterizerState state;
    state.EnableCull = false;
    _GM->RHISetRasterizerState(state);
    _GM->GLDEBUG();
    
    /*****************************/

#if defined(_DEBUG) && defined(DIRECT_DISPATCH)
    std::cout << "Ocean surface patch num: " << m_PatchNumToDraw << std::endl;
#endif // _DEBUG

    HugeSurfacePass::Draw(frame);

    state.EnableCull = true;
    _GM->RHISetRasterizerState(state);
}


#ifdef _DEBUG

void und::OceanSurfacePass::SwitchPauseOceanSurface()
{
    PauseOceanSurface = !PauseOceanSurface;
}
void und::OceanSurfacePass::SwitchInputColors()
{
    InputColors = !InputColors;
}

#endif // _DEBUG