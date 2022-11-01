#include "BaseSceneNode.h"

namespace und {
    thread_local int32_t indent = 0;
    std::ostream &operator<<(std::ostream &out, const BaseSceneNode &node)
    {
        
        indent++;

        out << std::string(indent, ' ') << node.m_NodeType << " Name: " << node.m_strName << std::endl;
        out << std::string(indent, ' ') << "-----------------------------------------------" << std::endl;
        node.dump(out);
        out << std::string(indent, ' ') << "-----------------------------------------------" << std::endl;
        out << std::endl;

        for (auto &sub_node : node.m_Children) {
            out << *sub_node << std::endl;
        }

        out << node.m_RelativeTransform << std::endl;

        indent--;

        return out;
    }
    void BaseSceneNode::AppendChild(std::shared_ptr<BaseSceneNode> sub_node)
    {
        sub_node->m_Parent = this;
        m_Children.push_back(sub_node);
        sub_node->bNodeToWorldUpdated = false;
        sub_node->UpdateNodeToWorld();
    }
    void BaseSceneNode::SetRelativeTransform(const SceneObjectTransform &transform)
    {
        m_RelativeTransform = transform;
        bNodeToWorldUpdated = false;
        UpdateNodeToWorld();
    }
    SceneObjectTransform BaseSceneNode::GetRelativeTransform()
    {
        return m_RelativeTransform;
    }
    SceneObjectTransform BaseSceneNode::GetWorldTransform()
    {
        return m_WorldTransform;
    }
    glm::vec3 BaseSceneNode::GetNodeLocation()
    {
        return m_WorldTransform.GetTranslation();
    }
    void BaseSceneNode::MoveNodeTo(const glm::vec3 &position)
    {
        m_WorldTransform.SetTranslation(position);
        if (m_Parent)
            m_RelativeTransform = m_WorldTransform.CalcRelativeTransform(m_Parent->GetWorldTransform());
        else
            m_RelativeTransform = m_WorldTransform;
        bNodeToWorldUpdated = true;
        for (auto &child : m_Children)
            child->UpdateNodeToWorld();
    }
    void BaseSceneNode::MoveNode(const glm::vec3 &Delta, const Rotator &NewRotation)
    {
        MoveNode(Delta, NewRotation.ToQuat());
    }
    void BaseSceneNode::MoveNode(const glm::vec3 &Delta, const glm::quat &NewRotation)
    {
        m_WorldTransform.SetTranslation(m_WorldTransform.GetTranslation() + Delta);
        m_WorldTransform.SetRotation(NewRotation);
        if (m_Parent)
            m_RelativeTransform = m_WorldTransform.CalcRelativeTransform(m_Parent->GetWorldTransform());
        else
            m_RelativeTransform = m_WorldTransform;
        bNodeToWorldUpdated = true;
        for (auto &child : m_Children)
            child->UpdateNodeToWorld();
    }
    glm::vec3 BaseSceneNode::GetForwardVector()
    {
        return -GetWorldTransform().GetUnitAxis(CoordAxis::X);
    }
    glm::vec3 BaseSceneNode::GetUpVector()
    {
        return GetWorldTransform().GetUnitAxis(CoordAxis::Z);
    }
    glm::vec3 BaseSceneNode::GetRightVector()
    {
        return GetWorldTransform().GetUnitAxis(CoordAxis::Y);
    }
    void BaseSceneNode::UpdateNodeToWorld()
    {
        if (m_Parent && !m_Parent->bNodeToWorldUpdated)
        {
            m_Parent->UpdateNodeToWorld();
            if (bNodeToWorldUpdated)
                return;
        }

        bNodeToWorldUpdated = true;
        if (m_Parent)
            m_WorldTransform = m_RelativeTransform * m_Parent->GetWorldTransform();
        else
            m_WorldTransform = m_RelativeTransform;
        for (auto &child : m_Children)
            child->UpdateNodeToWorld();
    }
    std::list<std::shared_ptr<BaseSceneNode>> &BaseSceneNode::GetChildren()
    {
        return m_Children;
    }
}

