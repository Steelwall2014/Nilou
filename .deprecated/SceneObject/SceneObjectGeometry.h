#pragma once
#include "SceneObjectMesh.h"

namespace und {
    class SceneObjectGeometry : public BaseSceneObject
    {
    protected:
        std::vector<std::shared_ptr<SceneObjectMesh>> m_Mesh;
    public:
        SceneObjectGeometry() : BaseSceneObject(SceneObjectType::kSceneObjectTypeGeometry) {};

        void AddMesh(std::shared_ptr<SceneObjectMesh> &mesh);

        std::vector<std::shared_ptr<SceneObjectMesh>> &GetMeshes();

        virtual void dump(std::ostream &out);
    };
}