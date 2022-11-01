#include "ForwardRenderPass.h"
#include "Common/ShaderManager.h"
#include "DynamicRHI.h"
#include "Common/SceneManager.h"
#include "OpenGL/OpenGLUtils.h"

namespace und {
    int ForwardRenderPass::Initialize(FrameVariables &frame)
    {
        int geom_count = g_pSceneManager->GetSceneGeometryNodeCount();
        for (int i = 0; i < geom_count; i++)
        {
            auto geom_node = g_pSceneManager->GetSceneGeometryNode(i).lock();
            glm::mat4 modelMatrix = geom_node->GetWorldTransform().ToMatrix();
            auto geom_obj = geom_node->GetSceneObjectRef();

            for (auto &mesh_obj : geom_obj->GetMeshes())
            {
                DrawBatchContext dbc = CreateDBCFromMesh(*mesh_obj);
                dbc.modelMatrix = modelMatrix;
                dbc.node = geom_node;
                frame.batchContexts.push_back(dbc);
            }

        }
        return 0;
    }

    void ForwardRenderPass::Draw(FrameVariables &frame)
    {
        _GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Forward default"));

        SetPerFrameShaderParameters(frame.frameContext);

        RHITextureParams params;
        params.Wrap_S = ETextureWrapModes::TW_Clamp;
        params.Wrap_T = ETextureWrapModes::TW_Clamp;
        _GM->RHIBindTexture("shadowMap", frame.frameContext.shadowMapArray, params);

        for (int i = 0; i < frame.batchContexts.size(); i++)
        {
            auto &context = frame.batchContexts[i];
            _GM->RHISetShaderParameter("model", context.modelMatrix);
            auto set_texture = [](const char *name, RHITexture2DRef texture, RHITexture2DRef default_texture) {
                texture = texture != nullptr ? texture : default_texture;
                _GM->RHIBindTexture(name, texture);
            };
            set_texture("baseColorMap", context.material.baseColorTexture, MaterialTextures::DefaultMaterial->baseColorTexture);

            set_texture("emissiveMap", context.material.emissiveTexture, MaterialTextures::DefaultMaterial->emissiveTexture);

            set_texture("normalMap", context.material.normalTexture, MaterialTextures::DefaultMaterial->normalTexture);

            set_texture("occlusionMap", context.material.occlusionTexture, MaterialTextures::DefaultMaterial->occlusionTexture);

            set_texture("roughnessMetallicMap", context.material.roughnessMetallicTexture, MaterialTextures::DefaultMaterial->roughnessMetallicTexture);

            _GM->RHIDrawVertexArray(context.vao, frame.frameContext.renderTarget);
        }
    }


}
