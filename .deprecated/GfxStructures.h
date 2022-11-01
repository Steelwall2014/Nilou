#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "Common/SceneNode/SceneGeometryNode.h"
#include "RHIResources.h"

namespace und {
    // class OpenGLVertexArrayObject;
    // class OpenGLIndexArrayBuffer;
    // class OpenGLTexture2DArray;
    // class OpenGLTextureImage2D;
    // class OpenGLTexture2D;
    // class OpenGLFramebuffer;

    // typedef std::shared_ptr<OpenGLVertexArrayObject> OpenGLVertexArrayObjectRef;
    // typedef std::shared_ptr<OpenGLIndexArrayBuffer> OpenGLIndexArrayBufferRef;
    // typedef std::shared_ptr<OpenGLTexture2DArray> OpenGLTexture2DArrayRef;
    // typedef std::shared_ptr<OpenGLTextureImage2D> OpenGLTextureImage2DRef;
    // typedef std::shared_ptr<OpenGLTexture2D> OpenGLTexture2DRef;
    // typedef std::shared_ptr<OpenGLFramebuffer> OpenGLFramebufferRef;

    enum class LightType
    {
        Point = 0, 
        Spot = 1, 
        Directional = 2, 
    };
    class Frustum
    {
    public:
        glm::vec4 Planes[6];
        Frustum(const glm::mat4 &view, const glm::mat4 &projection);
        Frustum() {};
        bool IsOutSideFrustum(glm::vec3 position);
    private:
        bool IsOutSidePlane(glm::vec4 plane, glm::vec3 position);
    };
    struct Light {
        LightType       lightType;
        glm::vec3       lightPosition;
        glm::vec4       lightColor;
        glm::vec3       lightDirection;
        float           lightIntensity;
        AttenCurveType  lightDistAttenCurveType;
        float           lightDistAttenCurveParams[5];
        AttenCurveType  lightAngleAttenCurveType;
        float           lightAngleAttenCurveParams[5];
        bool            lightCastShadow;
        int             lightShadowMapLayerIndex;
        glm::mat4       lightVP;
        float           nearClipDistance;
        float           farClipDistance;
        glm::ivec2      shadowMapResolution;

        Light()
        {
            lightPosition = { 0.0f, 0.0f, 0.0f };
            lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            lightDirection = { 0.0f, 0.0f, -1.0f };
            lightIntensity = 300.f;
            lightDistAttenCurveType = AttenCurveType::kNone;
            lightAngleAttenCurveType = AttenCurveType::kNone;
            for (int i = 0; i < 5; i++)
            {
                lightAngleAttenCurveParams[i] = 0.f;
                lightDistAttenCurveParams[i] = 0.f;
            }
            lightCastShadow = true;
            lightShadowMapLayerIndex = -1;
            nearClipDistance = 1.f;
            farClipDistance = 100.f;
            shadowMapResolution = glm::ivec2(1024, 1024);
        }
    };
    struct MaterialTextures
    {
        RHITexture2DRef roughnessMetallicTexture;
        RHITexture2DRef occlusionTexture;
        RHITexture2DRef normalTexture;
        RHITexture2DRef baseColorTexture;
        RHITexture2DRef emissiveTexture;
        MaterialTextures();
        static MaterialTextures *DefaultMaterial;
    };
    
    struct DrawFrameContext {
        glm::mat4           viewMatrix;
        glm::mat4           projectionMatrix;
        std::vector<Light>  lights;
        glm::vec3           cameraPosition;
        glm::vec3           cameraForward;
        glm::vec3           cameraUp;
        glm::vec3           cameraRight;
        float               cameraFOVy;
        float               cameraAspect;
        float               cameraNearClip;
        float               cameraFarClip;
        Frustum             cameraFrustum;

        RHITexture2DArrayRef shadowMapArray;
        RHIFramebufferRef    renderTarget;
    };

    struct DrawBatchContext {
        RHIVertexArrayObjectRef  vao;
        glm::mat4 modelMatrix;
        std::shared_ptr<SceneGeometryNode> node;
        MaterialTextures material;
    };

    struct FrameVariables {
        DrawFrameContext frameContext;
        std::vector<DrawBatchContext> batchContexts;
        FrameVariables() {}
        FrameVariables(const FrameVariables &other) = delete;
    };
}