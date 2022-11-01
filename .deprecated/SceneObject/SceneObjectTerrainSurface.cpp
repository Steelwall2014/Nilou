#include <glad/glad.h>
#include "SceneObjectTerrainSurface.h"
#include "Common/SceneObject/SceneObjectMesh.h"

namespace und {
	und::SceneObjectTerrainSurface::SceneObjectTerrainSurface(
		std::shared_ptr<SceneObjectTexture> HeightMap, float PixelMeterSize,
		std::shared_ptr<SceneObjectTerrainMaterial> TerrainMaterial, float MaterialMeterSize)
		: SceneObjectHugeSurface(kSceneObjectTtypeTerrainSurface)
		, TerrainMaterial(TerrainMaterial)
		, PixelMeterSize(PixelMeterSize)
		, MaterialMeterSize(MaterialMeterSize)
	{
		auto img = HeightMap->GetTextureImage();
		if (img->type == GL_SHORT)
		{
			std::shared_ptr<und::Image> float_img = std::make_shared<und::Image>();
			float_img->Width = img->Width;
			float_img->Height = img->Height;
			float_img->Channel = img->Channel;
			float_img->data_size = img->data_size * 2;
			float_img->type = GL_FLOAT;
			float *float_data = new float[float_img->Width * float_img->Height * float_img->Channel];
			short *short_data = (short *)img->data;
			for (int i = 0; i < float_img->Width * float_img->Height * float_img->Channel; i++)
				float_data[i] = short_data[i];
			float_img->data = (unsigned char *)float_data;
			img = float_img;
		}

		this->HeightMap = std::make_shared<SceneObjectTexture>(img);
		// !!!注意!!!  暂时只支持正方形的高度图
		WorldMeterSize = img->Width * PixelMeterSize;		
		HeightMapMeterSize = WorldMeterSize;
		InitializeQuadTree(6, 5, WorldMeterSize / 5.0);
	}
}