#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/Image.h"
#include "RHIDefinitions.h"

namespace und {
	class UTexture
	{
	private:
		std::shared_ptr<Image> m_Image;
	public:
		ETextureFilters Min_Filter;
		ETextureFilters Mag_Filter;
		ETextureWrapModes Wrap_S;
		ETextureWrapModes Wrap_T;
		UTexture();
		UTexture(std::shared_ptr<Image> img);
		UTexture(const char *filepath);
		UTexture(const std::string filepath);
		bool LoadTexture(const char *filepath);
		std::shared_ptr<Image> GetTextureImage();
	};
}