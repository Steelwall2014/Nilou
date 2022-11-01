#pragma once
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "SceneObject.h"

namespace und {
    class BaseSceneNode {
    protected:
        std::string m_NodeType = "BaseSceneNode";
        std::string m_strName;
        BaseSceneNode *m_Parent=nullptr;
        std::list<std::shared_ptr<BaseSceneNode>> m_Children;
        // m_RelativeTransform表达的是相对变换
        // 因此要先应用m_RelativeTransform再应用上一级的变换才是正确的m_WorldTransform
        SceneObjectTransform m_RelativeTransform;          
        SceneObjectTransform m_WorldTransform;
        bool bNodeToWorldUpdated = false;

    protected:
        virtual void dump(std::ostream &out) const {};

    public:
        BaseSceneNode() {};
        BaseSceneNode(const char *name) { m_strName = name; };
        BaseSceneNode(const std::string &name) { m_strName = name; };
        BaseSceneNode(const std::string &&name) { m_strName = std::move(name); };
        virtual ~BaseSceneNode() {};

        void AppendChild(std::shared_ptr<BaseSceneNode> sub_node);

        // 设置局部变换
        void SetRelativeTransform(const SceneObjectTransform &transform);

        // 获取局部变换
        SceneObjectTransform GetRelativeTransform();

        // 获取相对于世界参考系的变换
        SceneObjectTransform GetWorldTransform();

        // 获取相对于世界参考系的位置，通过调用GetWorldTransform实现
        glm::vec3 GetNodeLocation();

        void MoveNodeTo(const glm::vec3 &position);
        // 在世界参考系中移动node，将node在世界参考系中的旋转设置为NewRotation
        void MoveNode(const glm::vec3 &Delta, const Rotator &NewRotation);
        void MoveNode(const glm::vec3 &Delta, const glm::quat &NewRotation);

        // 获取这个节点的forward、up和right向量，在世界坐标系
        glm::vec3 GetForwardVector();
        glm::vec3 GetUpVector();
        glm::vec3 GetRightVector();
        //SceneObjectTransform GetRelativeTransform();
        void UpdateNodeToWorld();
        friend std::ostream &operator<<(std::ostream &out, const BaseSceneNode &node);
        std::list<std::shared_ptr<BaseSceneNode>> &GetChildren();
    };

    template <typename T>
    class SceneNode : public BaseSceneNode {
    protected:
        std::shared_ptr<T> m_pSceneObject;

    protected:
        virtual void dump(std::ostream &out) const
        {
            if (m_pSceneObject)
                m_pSceneObject->dump(out);
        };

    public:
        using BaseSceneNode::BaseSceneNode;
        SceneNode() = default;
        SceneNode(const std::shared_ptr<T> &object) { m_pSceneObject = object; }
        SceneNode(const std::shared_ptr<T> &&object) { m_pSceneObject = std::move(object); }

        void AddSceneObjectRef(const std::shared_ptr<T> &object) { m_pSceneObject = object; }
        void AddSceneObjectRef(const std::shared_ptr<T> &&object) { m_pSceneObject = std::move(object); }
        const std::shared_ptr<T> GetSceneObjectRef() { return m_pSceneObject; };

    };


    typedef BaseSceneNode SceneEmptyNode;
}
