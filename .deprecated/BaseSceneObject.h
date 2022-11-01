#pragma once
#include <cstdint>
#include <crossguid/guid.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


namespace und {

    typedef float (*AttenFunc)(float /* Distance or Angle */);


    enum SceneObjectType {
        kSceneObjectTypeMesh,
        kSceneObjectTypeMaterial,
        kSceneObjectTypeTexture,
        kSceneObjectTypeLight,
        kSceneObjectTypePointLight,
        kSceneObjectTypeSpotLight,
        kSceneObjectTypeDirectionalLight,
        kSceneObjectTypeCamera,
        kSceneObjectTypeAnimator,
        kSceneObjectTypeClip,
        kSceneObjectTypeVertexArray,
        kSceneObjectTypeIndexArray,
        kSceneObjectTypeGeometry,
        kSceneObjectTtypeSkybox,
        kSceneObjectTtypeHugeSurface,
        kSceneObjectTtypeOceanSurface,
        kSceneObjectTtypeTerrainSurface,
        kSceneObjectTtypeAtmosphere,
        kSceneObjectTtypeWaterbody
    };


    class BaseSceneObject
    {
    protected:
        xg::Guid m_Guid;
        SceneObjectType m_Type;
        BaseSceneObject(SceneObjectType type);
        BaseSceneObject(xg::Guid &guid, SceneObjectType type);
        BaseSceneObject(xg::Guid &&guid, SceneObjectType type);
        BaseSceneObject(BaseSceneObject &&obj);
        BaseSceneObject &operator=(BaseSceneObject &&obj);

    private:
        BaseSceneObject() = delete;
        BaseSceneObject(BaseSceneObject &obj) = delete;
        BaseSceneObject &operator=(BaseSceneObject &obj) = delete;

    public:
        const xg::Guid &GetGuid() const;
        SceneObjectType GetType();
        virtual void dump(std::ostream &out);
        //friend std::ostream &operator<<(std::ostream &out, const BaseSceneObject &obj);
    };
}
