#pragma once
#include "Common/StaticMeshResources.h"
#include <memory>
#include <Common/Scene.h>
#include <tinygltf/tiny_gltf.h>

namespace nilou {
	class GLTFParser
	{
	public:
		// std::unique_ptr<Scene> Parse(tinygltf::Model &model);
		std::vector<std::shared_ptr<UStaticMesh>> ParseToStaticMeshes(tinygltf::Model &model);
		std::vector<std::shared_ptr<FMaterial>> ParseToMaterials(tinygltf::Model &model);
	private:
		// tinygltf::Model *m_pTempModel = nullptr;
		// Scene *m_pTempScene = nullptr;
		// std::vector<std::shared_ptr<SceneObjectTexture>> m_textures;
		// void build_scene_graph_recursive(std::vector<int> &children, BaseSceneNode &root_node);
		// std::shared_ptr<SceneObjectTexture> ConvertGLTFTextureToSceneObject(tinygltf::Texture &gltf_texture);
		// std::shared_ptr<SceneObjectMaterial> ConvertGLTFMaterialToSceneObject(tinygltf::Material &gltf_material);
		// std::shared_ptr<BaseSceneNode> ConvertGLTFNodeToSceneNode(tinygltf::Node &root);
	};

	extern GLTFParser *g_pGLTFParser;
}