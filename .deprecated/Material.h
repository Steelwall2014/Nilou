#pragma once
#include "Common/ParameterValueMap.h"
#include "Common/UTexture.h"

namespace und {

    class UMaterial
    {

    public:

        UMaterial() {}
        explicit UMaterial(const char *name) {
            Name = name;
        }
        explicit UMaterial(const std::string &name)
            : UMaterial() {
            Name = name;
        }
        explicit UMaterial(std::string &&name) {
            Name = std::move(name);
        }

        const std::string &GetName() const { return Name; }
        //const Color &GetSpecularColor() const { return Specular; }
        //const Parameter &GetSpecularPower() const {
        //    return SpecularPower;
        //}
        //const Parameter &GetMetallic() const { return Metallic; }
        //const Parameter &GetRoughness() const { return Roughness; }
        //const Parameter &GetAO() const { return AmbientOcclusion; }
        //const Parameter &GetHeight() const { return Height; }
        const FParameterValueMap<glm::vec4> &GetBaseColor() const { return BaseColor; }
        const FParameterValueMap<glm::vec3> &GetOcclusion() const { return Occlusion; }
        const FParameterValueMap<glm::vec3> &GetRoughnessMetallic() { return Roughness_Metallic; };
        const FParameterValueMap<glm::vec3> &GetNormal() const { return Normal; }
        const FParameterValueMap<glm::vec3> &GetEmissive() const { return Emissive; }

        void SetBaseColor(const glm::vec4 &base_color)
        {
            BaseColor = FParameterValueMap<glm::vec4>(base_color);
        }
        void SetBaseColor(const std::shared_ptr<UTexture> &base_color)
        {
            BaseColor = FParameterValueMap<glm::vec4>(base_color);
        }

        void SetOcclusion(const glm::vec3 &occlusion)
        {
            Occlusion = FParameterValueMap<glm::vec3>(occlusion);
        }
        void SetOcclusion(const std::shared_ptr<UTexture> &occlusion)
        {
            Occlusion = FParameterValueMap<glm::vec3>(occlusion);
        }

        void SetRoughnessMetallic(const glm::vec3 &roughness_metallic) 
        {
            Roughness_Metallic = FParameterValueMap<glm::vec3>(roughness_metallic);
        }
        void SetRoughnessMetallic(const std::shared_ptr<UTexture> &roughness_metallic)
        {
            Roughness_Metallic = FParameterValueMap<glm::vec3>(roughness_metallic);
        }

        void SetNormal(const glm::vec3 &normal)
        {
            Normal = FParameterValueMap<glm::vec3>(normal);
        }
        void SetNormal(const std::shared_ptr<UTexture> &normal)
        {
            Normal = FParameterValueMap<glm::vec3>(normal);
        }   

        void SetEmissive(const glm::vec3 &emissive)
        {
            Emissive = FParameterValueMap<glm::vec3>(emissive);
        }
        void SetEmissive(const std::shared_ptr<UTexture> &emissive)
        {
            Emissive = FParameterValueMap<glm::vec3>(emissive);
        }

    protected:

        std::string Name;
        FParameterValueMap<glm::vec4> BaseColor;
        FParameterValueMap<glm::vec3> Occlusion;
        FParameterValueMap<glm::vec3> Roughness_Metallic;
        FParameterValueMap<glm::vec3> Normal;
        FParameterValueMap<glm::vec3> Emissive;
        //Parameter Metallic;
        //Parameter Roughness;
        //Normal Normal;
        //Color Specular;
        //Parameter SpecularPower;
        //Parameter AmbientOcclusion;
        //Color Opacity;
        //Color Transparency;
        //Color Emission;
        //Parameter Height;


        //friend std::ostream &operator<<(std::ostream &out,
        //    const UMaterial &obj);
    };

    class SceneObjectTerrainMaterial : public BaseSceneObject
    {
    protected:
        std::string Name;
        FParameterValueMap<glm::vec4> BaseColor;
        FParameterValueMap<glm::vec3> Roughness;
        FParameterValueMap<glm::vec3> Normal;

    public:
        SceneObjectTerrainMaterial()
            : BaseSceneObject(SceneObjectType::kSceneObjectTypeMaterial) {}
        explicit SceneObjectTerrainMaterial(const char *name) : SceneObjectTerrainMaterial() {
            Name = name;
        }
        explicit SceneObjectTerrainMaterial(const std::string &name)
            : SceneObjectTerrainMaterial() {
            Name = name;
        }
        explicit SceneObjectTerrainMaterial(std::string &&name) : SceneObjectTerrainMaterial() {
            Name = std::move(name);
        }

        const std::string &GetName() const { return Name; }
        const FParameterValueMap<glm::vec4> &GetBaseColor() const { return BaseColor; }
        const FParameterValueMap<glm::vec3> &GetRoughness() { return Roughness; };
        const FParameterValueMap<glm::vec3> &GetNormal() const { return Normal; }

        void SetBaseColor(const glm::vec4 &base_color)
        {
            BaseColor = FParameterValueMap<glm::vec4>(base_color);
        }
        void SetBaseColor(const std::shared_ptr<UTexture> &base_color)
        {
            BaseColor = FParameterValueMap<glm::vec4>(base_color);
        }

        void SetRoughness(const glm::vec3 &roughness)
        {
            Roughness = FParameterValueMap<glm::vec3>(roughness);
        }
        void SetRoughness(const std::shared_ptr<UTexture> &roughness)
        {
            Roughness = FParameterValueMap<glm::vec3>(roughness);
        }

        void SetNormal(const glm::vec3 &normal)
        {
            Normal = FParameterValueMap<glm::vec3>(normal);
        }
        void SetNormal(const std::shared_ptr<UTexture> &normal)
        {
            Normal = FParameterValueMap<glm::vec3>(normal);
        }
    };
}