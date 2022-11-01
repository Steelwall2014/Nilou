#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/QuadTree/QuadTree.h"
#include "Common/SceneObject/SceneObjectTexture.h"

namespace und {
	class SceneObjectMesh;
	class SceneObjectHugeSurface : public BaseSceneObject
	{
	protected:
		glm::vec2 *pos_data;
		glm::vec2 *uv_data;
		unsigned int *indices_data;
		SceneObjectHugeSurface();
		SceneObjectHugeSurface(SceneObjectType type);
		void InitializeQuadTree(unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize);
	public:
		unsigned int PatchGridSideNum;				// patch一边有多少个顶点
		float		 PatchOriginalGridMeterSize;
		std::shared_ptr<QuadTree>			QTree;
		std::shared_ptr<SceneObjectMesh>	PatchMesh;
		float								WorldMeterSize;
		float								HeightMapMeterSize;
		virtual ~SceneObjectHugeSurface();
	};
}