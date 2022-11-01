#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/Image.h"
#include "RHIDefinitions.h"

namespace und {
	class SceneObjectTexture : public BaseSceneObject
	{
	private:
		std::shared_ptr<Image> m_Image;
	public:
		ETextureFilters Min_Filter;
		ETextureFilters Mag_Filter;
		ETextureWrapModes Wrap_S;
		ETextureWrapModes Wrap_T;
		SceneObjectTexture();
		SceneObjectTexture(std::shared_ptr<Image> img);
		SceneObjectTexture(const char *filepath);
		SceneObjectTexture(const std::string filepath);
		bool LoadTexture(const char *filepath);
		std::shared_ptr<Image> GetTextureImage();
	};
}