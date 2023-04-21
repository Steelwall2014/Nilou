#include <regex>

#include "Cesium3DTileComponent.h"
#include "Material.h"
#include "Texture2D.h"
#include "Common/Asset/AssetLoader.h"



namespace nilou {


    using namespace Cesium3DTilesetSelection;

    void BoundingVolumeConvert(FOrientedBoundingBox &Box, tiny3dtiles::BoundingVolume &BoundingVolume, const glm::dmat4 &transform)
    {
        auto &box = BoundingVolume.box;
        dvec3 center = transform * dvec4(box[0], box[1], box[2], 1.0);
        dmat3 halfAxis = dmat3(transform) * dmat3(box[3], box[4], box[5], box[6], box[7], box[8], box[9], box[10], box[11]);
        Box = FOrientedBoundingBox(center, halfAxis);
    }

    bool Cesium3DTile::HasRendableContent() const
    {
        return Content.B3dm.header_loaded();
    }

    std::shared_ptr<Cesium3DTile> Cesium3DTile::BuildTile(
        std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform, ETileGltfUpAxis TileGltfUpAxis)
    {
        std::shared_ptr<Cesium3DTile> InternalTile = std::make_shared<Cesium3DTile>();
        InternalTile->TileGltfUpAxis = TileGltfUpAxis;
        InternalTile->GeometricError = Tile->geometricError;
        InternalTile->Refine = Tile->refine;
        InternalTile->Transform = parentTransform * Tile->transform;
        BoundingVolumeConvert(InternalTile->BoundingVolume, Tile->boundingVolume, InternalTile->Transform);
        InternalTile->Content.URI = Tile->content.uri;
        // InternalTile->TransformRHI = CreateUniformBuffer<FPrimitiveShaderParameters>();
        if (!Tile->content.b3dm.featureTableJSON.empty())
        {
            nlohmann::json json;
            std::stringstream stream(Tile->content.b3dm.featureTableJSON);
            stream >> json;
            if (json.is_object() && json.contains("RTC_CENTER"))
            {
                nlohmann::json rtc_center = json["RTC_CENTER"];
                if (rtc_center.is_array() && 
                    rtc_center.size() == 3)
                {
                    InternalTile->RtcCenter.x = rtc_center.at(0).get<double>();
                    InternalTile->RtcCenter.y = rtc_center.at(1).get<double>();
                    InternalTile->RtcCenter.z = rtc_center.at(2).get<double>();
                }
            }
        }
        

        if (Tile->content.external_tileset != nullptr)
        {
            std::shared_ptr<Cesium3DTile> ExternalRoot = Cesium3DTileset::Build(Tile->content.external_tileset, InternalTile->Transform)->Root;
            InternalTile->Children.push_back(ExternalRoot);
        }
        else 
        {
            InternalTile->Content.B3dm = Tile->content.b3dm;
        }

        for (auto &child : Tile->children)
        {
            InternalTile->Children.push_back(BuildTile(child, InternalTile->Transform, TileGltfUpAxis));
        }

        return InternalTile;
    }

    std::shared_ptr<Cesium3DTileset> Cesium3DTileset::Build(
        std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform)
    {
        ETileGltfUpAxis TileGltfUpAxis = ETileGltfUpAxis::Y;
        if (Tileset->asset.find("gltfUpAxis") != Tileset->asset.end())
        {
            if (Tileset->asset["gltfUpAxis"] == "X")
                TileGltfUpAxis = ETileGltfUpAxis::X;
            else if (Tileset->asset["gltfUpAxis"] == "Y")
                TileGltfUpAxis = ETileGltfUpAxis::Y;
            else if (Tileset->asset["gltfUpAxis"] == "Z")
                TileGltfUpAxis = ETileGltfUpAxis::Z;
        }
        std::shared_ptr<Cesium3DTileset> InternalTileset = std::make_shared<Cesium3DTileset>();
        InternalTileset->Asset = Tileset->asset;
        InternalTileset->GeometricError = Tileset->geometricError;
        InternalTileset->Root = Cesium3DTile::BuildTile(Tileset->root, parentTransform, TileGltfUpAxis);
        return InternalTileset;
    }
}