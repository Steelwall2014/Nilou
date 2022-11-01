#include "BaseSceneObject.h"

namespace und {
    const char *SceneObjectType_String[] = {
        "MESH",
        "MATL",
        "TXTU",
        "LGHT",
        "CAMR",
        "ANIM",
        "CLIP",
        "VARR",
        "VARR",
        "GEOM"
    };
    //BaseSceneObject::BaseSceneObject(SceneObjectType type) : m_Type(type) { m_Guid = xg::newGuid(); }

    BaseSceneObject::BaseSceneObject(SceneObjectType type) 
        : m_Type(type) { m_Guid = xg::newGuid(); }
    BaseSceneObject::BaseSceneObject(xg::Guid &guid, SceneObjectType type) 
        : m_Guid(guid), m_Type(type) {}
    BaseSceneObject::BaseSceneObject(xg::Guid &&guid, SceneObjectType type) 
        : m_Guid(std::move(guid)), m_Type(type) {}
    BaseSceneObject::BaseSceneObject(BaseSceneObject &&obj) 
        : m_Guid(std::move(obj.m_Guid)), m_Type(obj.m_Type) {}
    BaseSceneObject &BaseSceneObject::operator=(BaseSceneObject &&obj) 
    { 
        this->m_Guid = std::move(obj.m_Guid); 
        this->m_Type = obj.m_Type; 
        return *this; 
    }

    const xg::Guid &BaseSceneObject::GetGuid() const
    {
        return m_Guid;
    }

    SceneObjectType BaseSceneObject::GetType()
    {
        return m_Type;
    }

    extern thread_local int32_t indent;
    void BaseSceneObject::dump(std::ostream &out)
    {
        out << std::string(indent, ' ') << "|" << "SceneObject " << SceneObjectType_String[this->m_Type] << " GUID: " << this->m_Guid << std::endl;

    }

    //std::ostream &operator<<(std::ostream &out, const BaseSceneObject &obj)
    //{
    //    out << "SceneObject" << std::endl;
    //    out << "-----------" << std::endl;
    //    out << "GUID: " << obj.m_Guid << std::endl;
    //    out << "Type: " << obj.m_Type << std::endl;

    //    return out;
    //}

}