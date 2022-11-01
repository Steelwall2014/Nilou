#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/ParameterValueMap.h"

namespace und {
    class SceneObjectMaterial : public BaseSceneObject {
    protected:
        std::string m_Name;
        ParameterValueMap<glm::vec4> m_BaseColor;
        ParameterValueMap<glm::vec3> m_Occlusion;
        ParameterValueMap<glm::vec3> m_Roughness_Metallic;
        ParameterValueMap<glm::vec3> m_Normal;
        ParameterValueMap<glm::vec3> m_Emissive;
        //Parameter m_Metallic;
        //Parameter m_Roughness;
        //Normal m_Normal;
        //Color m_Specular;
        //Parameter m_SpecularPower;
        //Parameter m_AmbientOcclusion;
        //Color m_Opacity;
        //Color m_Transparency;
        //Color m_Emission;
        //Parameter m_Height;

    public:
        SceneObjectMaterial()
            : BaseSceneObject(SceneObjectType::kSceneObjectTypeMaterial) {}
        explicit SceneObjectMaterial(const char *name) : SceneObjectMaterial() {
            m_Name = name;
        }
        explicit SceneObjectMaterial(const std::string &name)
            : SceneObjectMaterial() {
            m_Name = name;
        }
        explicit SceneObjectMaterial(std::string &&name) : SceneObjectMaterial() {
            m_Name = std::move(name);
        }

        const std::string &GetName() const { return m_Name; }
        //const Color &GetSpecularColor() const { return m_Specular; }
        //const Parameter &GetSpecularPower() const {
        //    return m_SpecularPower;
        //}
        //const Parameter &GetMetallic() const { return m_Metallic; }
        //const Parameter &GetRoughness() const { return m_Roughness; }
        //const Parameter &GetAO() const { return m_AmbientOcclusion; }
        //const Parameter &GetHeight() const { return m_Height; }
        const ParameterValueMap<glm::vec4> &GetBaseColor() const { return m_BaseColor; }
        const ParameterValueMap<glm::vec3> &GetOcclusion() const { return m_Occlusion; }
        const ParameterValueMap<glm::vec3> &GetRoughnessMetallic() { return m_Roughness_Metallic; };
        const ParameterValueMap<glm::vec3> &GetNormal() const { return m_Normal; }
        const ParameterValueMap<glm::vec3> &GetEmissive() const { return m_Emissive; }

        void SetBaseColor(const glm::vec4 &base_color)
        {
            m_BaseColor = ParameterValueMap<glm::vec4>(base_color);
        }
        void SetBaseColor(const std::shared_ptr<SceneObjectTexture> &base_color)
        {
            m_BaseColor = ParameterValueMap<glm::vec4>(base_color);
        }

        void SetOcclusion(const glm::vec3 &occlusion)
        {
            m_Occlusion = ParameterValueMap<glm::vec3>(occlusion);
        }
        void SetOcclusion(const std::shared_ptr<SceneObjectTexture> &occlusion)
        {
            m_Occlusion = ParameterValueMap<glm::vec3>(occlusion);
        }

        void SetRoughnessMetallic(const glm::vec3 &roughness_metallic) 
        {
            m_Roughness_Metallic = ParameterValueMap<glm::vec3>(roughness_metallic);
        }
        void SetRoughnessMetallic(const std::shared_ptr<SceneObjectTexture> &roughness_metallic)
        {
            m_Roughness_Metallic = ParameterValueMap<glm::vec3>(roughness_metallic);
        }

        void SetNormal(const glm::vec3 &normal)
        {
            m_Normal = ParameterValueMap<glm::vec3>(normal);
        }
        void SetNormal(const std::shared_ptr<SceneObjectTexture> &normal)
        {
            m_Normal = ParameterValueMap<glm::vec3>(normal);
        }   

        void SetEmissive(const glm::vec3 &emissive)
        {
            m_Emissive = ParameterValueMap<glm::vec3>(emissive);
        }
        void SetEmissive(const std::shared_ptr<SceneObjectTexture> &emissive)
        {
            m_Emissive = ParameterValueMap<glm::vec3>(emissive);
        }
        //friend std::ostream &operator<<(std::ostream &out,
        //    const SceneObjectMaterial &obj);
    };

    class SceneObjectTerrainMaterial : public BaseSceneObject
    {
    protected:
        std::string m_Name;
        ParameterValueMap<glm::vec4> m_BaseColor;
        ParameterValueMap<glm::vec3> m_Roughness;
        ParameterValueMap<glm::vec3> m_Normal;

    public:
        SceneObjectTerrainMaterial()
            : BaseSceneObject(SceneObjectType::kSceneObjectTypeMaterial) {}
        explicit SceneObjectTerrainMaterial(const char *name) : SceneObjectTerrainMaterial() {
            m_Name = name;
        }
        explicit SceneObjectTerrainMaterial(const std::string &name)
            : SceneObjectTerrainMaterial() {
            m_Name = name;
        }
        explicit SceneObjectTerrainMaterial(std::string &&name) : SceneObjectTerrainMaterial() {
            m_Name = std::move(name);
        }

        const std::string &GetName() const { return m_Name; }
        const ParameterValueMap<glm::vec4> &GetBaseColor() const { return m_BaseColor; }
        const ParameterValueMap<glm::vec3> &GetRoughness() { return m_Roughness; };
        const ParameterValueMap<glm::vec3> &GetNormal() const { return m_Normal; }

        void SetBaseColor(const glm::vec4 &base_color)
        {
            m_BaseColor = ParameterValueMap<glm::vec4>(base_color);
        }
        void SetBaseColor(const std::shared_ptr<SceneObjectTexture> &base_color)
        {
            m_BaseColor = ParameterValueMap<glm::vec4>(base_color);
        }

        void SetRoughness(const glm::vec3 &roughness)
        {
            m_Roughness = ParameterValueMap<glm::vec3>(roughness);
        }
        void SetRoughness(const std::shared_ptr<SceneObjectTexture> &roughness)
        {
            m_Roughness = ParameterValueMap<glm::vec3>(roughness);
        }

        void SetNormal(const glm::vec3 &normal)
        {
            m_Normal = ParameterValueMap<glm::vec3>(normal);
        }
        void SetNormal(const std::shared_ptr<SceneObjectTexture> &normal)
        {
            m_Normal = ParameterValueMap<glm::vec3>(normal);
        }
    };
}