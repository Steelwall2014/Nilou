#include "GfxStructures.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "OpenGL/OpenGLVertexArrayObject.h"
#include "OpenGL/OpenGLIndexArrayBuffer.h"
#include "OpenGL/OpenGLTexture.h"

void NormalizePlane(glm::vec4 &plane)
{
    float mag;
    mag = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    plane.x = plane.x / mag;
    plane.y = plane.y / mag;
    plane.z = plane.z / mag;
    plane.w = plane.w / mag;
}

und::Frustum::Frustum(const glm::mat4 &view, const glm::mat4 &projection)
{
    glm::mat4 VP = projection * view;
    VP = glm::transpose(VP);
    Planes[0].x = VP[3][0] + VP[0][0];
    Planes[0].y = VP[3][1] + VP[0][1];
    Planes[0].z = VP[3][2] + VP[0][2];
    Planes[0].w = VP[3][3] + VP[0][3];

    Planes[1].x = VP[3][0] - VP[0][0];
    Planes[1].y = VP[3][1] - VP[0][1];
    Planes[1].z = VP[3][2] - VP[0][2];
    Planes[1].w = VP[3][3] - VP[0][3];

    Planes[2].x = VP[3][0] - VP[1][0];
    Planes[2].y = VP[3][1] - VP[1][1];
    Planes[2].z = VP[3][2] - VP[1][2];
    Planes[2].w = VP[3][3] - VP[1][3];

    Planes[3].x = VP[3][0] + VP[1][0];
    Planes[3].y = VP[3][1] + VP[1][1];
    Planes[3].z = VP[3][2] + VP[1][2];
    Planes[3].w = VP[3][3] + VP[1][3];

    Planes[4].x = VP[3][0] + VP[2][0];
    Planes[4].y = VP[3][1] + VP[2][1];
    Planes[4].z = VP[3][2] + VP[2][2];
    Planes[4].w = VP[3][3] + VP[2][3];

    Planes[5].x = VP[3][0] - VP[2][0];
    Planes[5].y = VP[3][1] - VP[2][1];
    Planes[5].z = VP[3][2] - VP[2][2];
    Planes[5].w = VP[3][3] - VP[2][3];

}

bool und::Frustum::IsOutSideFrustum(glm::vec3 position)
{
    return IsOutSidePlane(Planes[0], position) ||
        IsOutSidePlane(Planes[1], position) ||
        IsOutSidePlane(Planes[2], position) ||
        IsOutSidePlane(Planes[3], position) ||
        IsOutSidePlane(Planes[4], position) ||
        IsOutSidePlane(Planes[5], position);
};

bool und::Frustum::IsOutSidePlane(glm::vec4 plane, glm::vec3 position)
{
    float a = glm::dot(glm::vec3(plane), position);
    return glm::dot(glm::vec3(plane), position) + plane.w < 0;
}

namespace und {
    MaterialTextures::MaterialTextures()
        : roughnessMetallicTexture(nullptr)
        , occlusionTexture(nullptr)
        , normalTexture(nullptr)
        , baseColorTexture(nullptr)
        , emissiveTexture(nullptr)
    {
    }
    MaterialTextures *MaterialTextures::DefaultMaterial = nullptr;
}


