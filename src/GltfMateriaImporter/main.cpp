#include <tinygltf/tiny_gltf.h>
#include <iostream>
#include <memory>

int main(int argc, char **argv)
{
    std::string in_gltf_path, out_glsl_path;
    const int argc_check = argc - 1;
    for (int i = 1; i < argc; i++)
    {
        if (!std::strcmp("-i", argv[i]) && i < argc_check)
            in_gltf_path = argv[++i];
        else if (!std::strcmp("-o", argv[i]) && i < argc_check)
            out_glsl_path = argv[++i];
    }
    
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, in_gltf_path);// "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\simple_mesh.gltf");
    if (!ret)
    {
        std::cout << err << ' ' << warn << std::endl;
        return 1;
    }

    std::string material_template = 
    
    for (tinygltf::Material &gltf_material : model.materials)
    {
        // if (gltf_material.emissiveTexture.index != -1)

    }
}