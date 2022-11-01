#pragma once
#include "Common/BaseSceneNode.h"

namespace und {
    class SceneGeometryNode : public SceneNode<SceneObjectGeometry>
    {
    protected:
        virtual void dump(std::ostream &out) const;

    public:
        SceneGeometryNode();
        using SceneNode::SceneNode;
        using SceneNode::AddSceneObjectRef;
    };
}
