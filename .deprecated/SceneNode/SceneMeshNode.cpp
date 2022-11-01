#include "SceneMeshNode.h"

und::SceneMeshNode::SceneMeshNode()
{
    m_NodeType = "SceneMeshNode";
}

void und::SceneMeshNode::dump(std::ostream &out) const
{
    SceneNode::dump(out);
};
