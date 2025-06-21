#pragma once
#if NILOU_ENABLE_3DTILES
#include <tiny3dtiles/tiny_3dtiles.h>
#include <tinygltf/tiny_gltf.h>

#include "Frustum.h"
#include "UniformBuffer.h"

namespace nilou {

    class UCesium3DTileComponent;

    BEGIN_UNIFORM_BUFFER_STRUCT(FGLTFMaterialBlock)
        SHADER_PARAMETER(vec4, baseColorFactor)
        SHADER_PARAMETER(vec3, emissiveFactor)
        SHADER_PARAMETER(float, metallicFactor)
        SHADER_PARAMETER(float, roughnessFactor)
    END_UNIFORM_BUFFER_STRUCT()

    struct GLTFParseResult
    {
        std::vector<std::shared_ptr<class UStaticMesh>> StaticMeshes;
        std::vector<std::shared_ptr<class UMaterialInstance>> Materials;
        std::vector<std::shared_ptr<class UTexture>> Textures;
        TUniformBufferRef<FGLTFMaterialBlock> UniformBuffer;
        void InitResource();
        // void ReleaseResource();
        ~GLTFParseResult();
    };

    namespace Cesium3DTilesetSelection {

        struct ViewState
        {
            FViewFrustum frustum;
            dvec3 cameraPosition;
            ivec2 resolution;
            double verticalFOV;
            /** The denominator for calculating screen space error */
            double sseDenominator; 
        };

        struct TileLoadingResult
        {
            class Cesium3DTile *Tile;
            std::shared_ptr<tinygltf::Model> model;
        };

        enum class ETileLoadingState
        {
            Unloaded,
            Unloading,
            Loading,
            Loaded
        };

        enum class ETileGltfUpAxis
        {
            X,
            Y,
            Z
        };

        class Cesium3DTile
        {
        public:
            friend class UCesium3DTilesetComponent;
            struct TileContent
            {
                std::string URI;
                tiny3dtiles::B3DM B3dm;
                std::shared_ptr<GLTFParseResult> Gltf = nullptr;
            };
            tiny3dtiles::Tile::Refinement Refine = tiny3dtiles::Tile::Refinement::REFINE;
            TileContent Content;
            std::vector<std::shared_ptr<Cesium3DTile>> Children;
            FOrientedBoundingBox BoundingVolume;    //  The bounding volume that has been transformed by parent transforms
            double GeometricError;
            glm::dmat4 Transform = glm::dmat4(1);   // The matrix that has been multiplied with parent transforms
            std::atomic<ETileLoadingState> LoadingState;
            ETileGltfUpAxis TileGltfUpAxis = ETileGltfUpAxis::Y;
            glm::dvec3 RtcCenter;

            std::shared_ptr<UCesium3DTileComponent> TileComponent = nullptr;

            bool IsLeaf() const { return Children.empty(); }

            bool HasRendableContent() const;

            static std::shared_ptr<Cesium3DTile> BuildTile(
                std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform, ETileGltfUpAxis TileGltfUpAxis);
            
        };

        class Cesium3DTileset
        {
        public:
            std::map<std::string, std::string> Asset;
            std::shared_ptr<Cesium3DTile> Root;
            double GeometricError;

            static std::shared_ptr<Cesium3DTileset> Build(
                std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform);
        };

    }

}
#endif
