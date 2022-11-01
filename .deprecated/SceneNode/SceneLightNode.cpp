#include "SceneLightNode.h"

und::SceneLightNode::SceneLightNode(glm::vec3 position, glm::vec3 target)
{
	m_NodeType = "SceneLightNode";
	m_Target = target;
	MoveNodeTo(position);
}

void und::SceneLightNode::SetTarget(glm::vec3 &target)
{
	m_Target = target;
}

glm::vec3 und::SceneLightNode::GetLightDir()
{
	return glm::normalize(m_Target - GetNodeLocation());
}
