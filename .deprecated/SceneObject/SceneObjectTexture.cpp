#include "SceneObjectTexture.h"
#include "Common/AssetLoader.h"
#include <tinygltf/tiny_gltf.h>

#include <glad/glad.h>

und::SceneObjectTexture::SceneObjectTexture()
	: BaseSceneObject(kSceneObjectTypeTexture)
	, Min_Filter(ETextureFilters::TF_Linear)
	, Mag_Filter(ETextureFilters::TF_Linear)
	, Wrap_S(ETextureWrapModes::TW_Repeat)
	, Wrap_T(ETextureWrapModes::TW_Repeat)
{
}

und::SceneObjectTexture::SceneObjectTexture(std::shared_ptr<Image> img)
	: BaseSceneObject(kSceneObjectTypeTexture)
	, Min_Filter(ETextureFilters::TF_Linear)
	, Mag_Filter(ETextureFilters::TF_Linear)
	, Wrap_S(ETextureWrapModes::TW_Repeat)
	, Wrap_T(ETextureWrapModes::TW_Repeat)
{
	m_Image = img;
}

und::SceneObjectTexture::SceneObjectTexture(const char *filepath)
	: BaseSceneObject(kSceneObjectTypeTexture)
	, Min_Filter(ETextureFilters::TF_Linear)
	, Mag_Filter(ETextureFilters::TF_Linear)
	, Wrap_S(ETextureWrapModes::TW_Repeat)
	, Wrap_T(ETextureWrapModes::TW_Repeat)
{
	LoadTexture(filepath);
}

und::SceneObjectTexture::SceneObjectTexture(const std::string filepath)
	: BaseSceneObject(kSceneObjectTypeTexture)
	, Min_Filter(ETextureFilters::TF_Linear)
	, Mag_Filter(ETextureFilters::TF_Linear)
	, Wrap_S(ETextureWrapModes::TW_Repeat)
	, Wrap_T(ETextureWrapModes::TW_Repeat)
{
	LoadTexture(filepath.c_str());
}

bool und::SceneObjectTexture::LoadTexture(const char *filepath)
{
	m_Image = g_pAssetLoader->SyncOpenAndReadImage(filepath);
	return true;
}

std::shared_ptr<und::Image> und::SceneObjectTexture::GetTextureImage()
{
	return m_Image;
}
