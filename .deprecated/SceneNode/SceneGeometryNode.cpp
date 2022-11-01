#include "SceneGeometryNode.h"

void und::SceneGeometryNode::dump(std::ostream &out) const
{
    SceneNode::dump(out);
}

und::SceneGeometryNode::SceneGeometryNode()
{
    m_NodeType = "SceneGeometryNode";
}
