#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/SceneObject/SceneObjectTexture.h"

namespace und {
	class SceneObjectSkybox : public BaseSceneObject
	{
	private:
		std::vector<std::shared_ptr<SceneObjectTexture>> m_Textures;
	public:
		SceneObjectSkybox(const std::vector<std::string> &faces);
		std::shared_ptr<SceneObjectTexture> GetTexture(int i);
	};

}