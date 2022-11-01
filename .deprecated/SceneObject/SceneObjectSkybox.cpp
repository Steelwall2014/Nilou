#include "SceneObjectSkybox.h"

und::SceneObjectSkybox::SceneObjectSkybox(const std::vector<std::string> &faces)
	: BaseSceneObject(kSceneObjectTtypeSkybox)
{
	for (auto &&face : faces)
	{
		std::shared_ptr<SceneObjectTexture> texture = std::make_shared<SceneObjectTexture>(face.c_str());
		m_Textures.push_back(texture);
	}
}

std::shared_ptr<und::SceneObjectTexture> und::SceneObjectSkybox::GetTexture(int i)
{
	return m_Textures[i];
}
