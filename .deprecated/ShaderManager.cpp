#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

#include "Shadinclude.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "ShaderManager.h"
#include "AssetLoader.h"

namespace und {
    ShaderManager *g_pShaderManager = new ShaderManager;
}
bool und::ShaderManager::LoadShader(const std::string vert_filepath, const std::string frag_filepath, const char *shader_name)
{
    return LoadShader(vert_filepath.c_str(), frag_filepath.c_str(), shader_name);
}
bool und::ShaderManager::LoadShader(const std::string comp_filepath, const char *shader_name)
{
    return LoadShader(comp_filepath.c_str(), shader_name);
}
bool und::ShaderManager::LoadShader(const char *vert_filepath, const char *frag_filepath, const char *shader_name)
{
    //char *vert_shader_code = g_pAssetLoader->SyncOpenAndReadText(vert_filepath);
    std::string vert_shader_code = Shadinclude::load(vert_filepath);
    und::RHIVertexShaderRef vert = GDynamicRHI->RHICreateVertexShader(vert_shader_code.c_str());
    //delete[] vert_shader_code;
    if (vert == nullptr)
    {
        std::cout << vert_filepath << " COMPILE ERROR" << std::endl;
        return false;
    }

    //char *frag_shader_code = g_pAssetLoader->SyncOpenAndReadText(frag_filepath);
    std::string frag_shader_code = Shadinclude::load(frag_filepath);
    und::RHIPixelShaderRef frag = GDynamicRHI->RHICreatePixelShader(frag_shader_code.c_str());
    //delete[] frag_shader_code;
    if (frag == nullptr)
    {
        std::cout << frag_filepath << " COMPILE ERROR" << std::endl;
        return false;
    }

    und::RHILinkedProgramRef program = GDynamicRHI->RHICreateLinkedProgram(vert, frag);
    if (program == nullptr)
        return false;

    m_Shaders[shader_name] = program;
    return true;
}
bool und::ShaderManager::LoadShader(const char *comp_filepath, const char *shader_name)
{
    //char *comp_shader_code = g_pAssetLoader->SyncOpenAndReadText(comp_filepath);
    std::string comp_shader_code = Shadinclude::load(comp_filepath);
    und::RHIComputeShaderRef comp = GDynamicRHI->RHICreateComputeShader(comp_shader_code.c_str());
    //delete[] comp_shader_code;

    if (comp == nullptr)
    {
        std::cout << comp_filepath << " COMPILE ERROR" << std::endl;
        return false;
    }

    und::RHILinkedProgramRef program = GDynamicRHI->RHICreateLinkedProgram(comp);
    if (program == nullptr)
        return false;

    m_Shaders[shader_name] = program;
    return true;
}

und::RHILinkedProgramRef und::ShaderManager::GetShaderByName(const char *shader_name)
{
    assert(m_Shaders.find(shader_name) != m_Shaders.end());
    return m_Shaders[shader_name];
}

int und::ShaderManager::Initialize()
{
    //ret = LoadShader(
    //    "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\Shaders\\phong.vert", 
    //    "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\Shaders\\phong.frag",
    //    "Phong");
    //ret |= LoadShader(
    //    "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\Shaders\\phong_spotlight.vert",
    //    "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\Shaders\\phong_spotlight.frag",
    //    "Phong Spotlight");
    bool ret = LoadShader(
        AssetLoader::AssetDir + "Shaders\\shadow_map.vert",
        AssetLoader::AssetDir + "Shaders\\shadow_map.frag",
        "Shadow map");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\debug_hud.vert",
        AssetLoader::AssetDir + "Shaders\\debug_hud_texture2d.frag",
        "Debug HUD Texture2D");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\debug_hud.vert",
        AssetLoader::AssetDir + "Shaders\\debug_hud_texture3d.frag",
        "Debug HUD Texture3D");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\debug_hud.vert",
        AssetLoader::AssetDir + "Shaders\\debug_hud_texture2darray.frag",
        "Debug HUD Texture2DArray");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\skybox.vert",
        AssetLoader::AssetDir + "Shaders\\skybox.frag",
        "Skybox");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\pbr.vert",
        AssetLoader::AssetDir + "Shaders\\pbr.frag",
        "Forward default");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_surface_shading.vert",
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_surface_shading.frag",
        "Ocean Surface Shading");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_create_gaussian.comp",
        "Ocean Gaussion");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_create_spectrums.comp",
        "Ocean Spectrums");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_fft.comp",
        "Ocean FFT");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_create_displacement.comp",
        "Ocean Displacement");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_create_normal_and_foam.comp",
        "Ocean Normal and Foam");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_patch.comp",
        "QuadTree Create Patch");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_lod_map.comp",
        "QuadTree Create LOD Map");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\seabed\\seabed_shading.vert",
        AssetLoader::AssetDir + "Shaders\\seabed\\seabed_shading.frag",
        "Seabed Shading");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\seabed\\seabed_normal.comp",
        "Seabed Normal");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_minmax_first.comp",
        "QuadTree Create MinMaxMap First");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_minmax.comp",
        "QuadTree Create MinMaxMap");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_minmax_node.comp",
        "QuadTree Create MinMaxMap Node");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\debug_boundingbox.vert",
        AssetLoader::AssetDir + "Shaders\\debug_boundingbox.frag",
        "Debug BoundingBox");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\quadtree\\quad_create_nodelist.comp",
        "QuadTree Create NodeList");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\deferred_rendering.vert",
        AssetLoader::AssetDir + "Shaders\\deferred_rendering.frag",
        "Deferred Rendering");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_calc_sum_slope.comp",
        "Calculate Slope Sum");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\ocean_surface\\ocean_calc_variance_slope.comp",
        "Calculate Slope Variance");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_transmittance_pre.comp",
        "Atmosphere Transmittance Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_scattering_pre.comp",
        "Atmosphere Scattering Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_direct_irradiance_pre.comp",
        "Atmosphere Direct Irradiance Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_scattering_density_pre.comp",
        "Atmosphere Scattering Density Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_indirect_irradiance_pre.comp",
        "Atmosphere Indirect Irradiance Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_multiscattering_pre.comp",
        "Atmosphere MultiScattering Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\atmosphere\\atmosphere_skymap.comp",
        "Atmosphere SkyMap");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\waterbody\\waterbody_scattering_pre.comp",
        "Waterbody SingleScattering Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\waterbody\\waterbody_multiscattering_pre.comp",
        "Waterbody MultiScattering Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\waterbody\\waterbody_scattering_density_pre.comp",
        "Waterbody ScatteringDensity Precompute");
    ret &= LoadShader(
        AssetLoader::AssetDir + "Shaders\\waterbody\\waterbody_scattering_synthesize_pre.comp",
        "Waterbody Scattering Synthesize Precompute");
    
    return !ret; 
}
