#pragma once
#include <tinygltf/tiny_gltf.h>

namespace nilou {
    class UTexture2D;
    class UMaterial;
    class UStaticMesh;

    class FGLTFImporter
    {
    public:
        static void Import(const std::string& InFilePath, const std::string& OutDirectory, std::vector<UTexture2D*>& Textures, std::vector<UMaterial*>& Materials, std::vector<UStaticMesh*>& Meshes);
    };

}