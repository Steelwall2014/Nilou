#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/QuadTree/QuadTree.h"
#include "Common/SceneObject/SceneObjectTexture.h"
#include "Common/SceneObject/SceneObjectMaterial.h"
#include "Common/SceneObject/SceneObjectHugeSurface.h"

namespace und {
	class SceneObjectMesh;
	class SceneObjectTerrainSurface : public SceneObjectHugeSurface
	{
	public:
		std::shared_ptr<SceneObjectTexture> HeightMap;
		std::shared_ptr<SceneObjectTerrainMaterial> TerrainMaterial;
		float		PixelMeterSize;
		float		MaterialMeterSize;
		SceneObjectTerrainSurface(
			std::shared_ptr<SceneObjectTexture> HeightMap, float PixelMeterSize, 
			std::shared_ptr<SceneObjectTerrainMaterial> TerrainMaterial, float MaterialMeterSize);
	};
}
