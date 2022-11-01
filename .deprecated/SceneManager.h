#pragma once
#include <memory>
#include "Interface/IRuntimeModule.h"
#include "Common/Scene.h"
#include "Common/GfxStructures.h"

namespace und {
    class OceanScatteringPrecomputeSubPass;
    class SceneManager : implements IRuntimeModule 
    {
    public:

        int Initialize() override;
        void Finalize() override;

        void Tick(double DeltaTime) override;

        int LoadScene(const char *scene_file_name);
        int LoadScene(const std::string scene_file_name);

        const std::shared_ptr<Scene> GetScene() const;
        const std::shared_ptr<Scene> GetSceneForPhysicalSimulation() const;

        void ResetScene();

        std::shared_ptr<BaseSceneNode> GetRootNode() const;
        //std::shared_ptr<SceneMeshNode> GetSceneMeshNode(const std::string &name) const;
        //std::shared_ptr<SceneObjectMesh> GetSceneMeshObject(const std::string &key) const;
        std::weak_ptr<SceneGeometryNode> GetSceneGeometryNode(int index) const;
        std::shared_ptr<SceneObjectGeometry> GetSceneGeometryObject(int index) const;
        int GetSceneGeometryNodeCount() const;
        void CalculateLightParams(FrameVariables &frame);
        FrameVariables frame;
    protected:
        bool LoadGLTFScene(const char *gltf_scene_file_name);
        void MoveSunHorizontal(float);
        void MoveSunVertical(float);
    protected:
        std::shared_ptr<Scene> m_pScene;
        std::shared_ptr<OceanScatteringPrecomputeSubPass> OceanScatteringPrecompute;
    };

    extern SceneManager *g_pSceneManager;
}