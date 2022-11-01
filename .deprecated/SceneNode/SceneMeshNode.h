#pragma once
#include "Common/BaseSceneNode.h"

namespace und {
	class SceneMeshNode : public SceneNode<SceneObjectMesh>
	{
    public:
        SceneMeshNode();
		virtual void dump(std::ostream &out) const;
	};
}