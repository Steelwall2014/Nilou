#include "glad/glad.h"
#include "SceneObjectHugeSurface.h"
#include "SceneObjectMesh.h"

namespace und {
    SceneObjectHugeSurface::SceneObjectHugeSurface()
        : BaseSceneObject(kSceneObjectTtypeHugeSurface)
    {
    }
    und::SceneObjectHugeSurface::SceneObjectHugeSurface(SceneObjectType type)
        : BaseSceneObject(type)
    {

    }
    void SceneObjectHugeSurface::InitializeQuadTree(unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize)
    {
        this->QTree = std::make_shared<und::QuadTree>(LODNum, TopLODNodeSideNum, TopLODNodeMeterSize);
        float patch_meter_size = QTree->GetOriginalPatchMeterSize();
        unsigned int N = PatchGridSideNum = 16 + 1;
        pos_data = new glm::vec2[N * N];
        uv_data = new glm::vec2[N * N];
        indices_data = new unsigned int[(N - 1) * (N - 1) * 6];
        float stride = PatchOriginalGridMeterSize = patch_meter_size / (N - 1);
        int index = 0;
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                pos_data[i * N + j].x = i * stride;
                pos_data[i * N + j].y = j * stride;
                uv_data[i * N + j].x = (float)(i + 0.5) / (float)(N - 1);
                uv_data[i * N + j].y = (float)(j + 0.5) / (float)(N - 1);
            }
        }
        index = 0;
        for (int i = 0; i < N - 1; i++)
        {
            for (int j = 0; j < N - 1; j++)
            {
                unsigned int lower_left_index = i * N + j;
                unsigned int upper_left_index = lower_left_index + 1;
                unsigned int lower_right_index = (i + 1) * N + j;
                unsigned int upper_right_index = lower_right_index + 1;

                indices_data[index++] = lower_right_index;
                indices_data[index++] = upper_right_index;
                indices_data[index++] = lower_left_index;

                indices_data[index++] = upper_right_index;
                indices_data[index++] = upper_left_index;
                indices_data[index++] = lower_left_index;

            }
        }
        PatchMesh = std::make_shared<SceneObjectMesh>();
        und::SceneObjectMesh::Primitive prim;

        und::SceneObjectMesh::AttriPointer pos_attr;
        pos_attr.name = "POSITION";
        pos_attr.normalized = false;
        pos_attr.offset = 0;
        pos_attr.pVertexAttriArray = pos_data;
        pos_attr.VertexAttriArraySize = N * N * sizeof(glm::vec2);
        pos_attr.size = 2;
        pos_attr.stride = 2 * sizeof(float);
        pos_attr.type = GL_FLOAT;
        prim.AddAttribute(pos_attr);

        und::SceneObjectMesh::AttriPointer uv_attr;
        uv_attr.name = "TEXCOORD_0";
        uv_attr.normalized = false;
        uv_attr.offset = 0;
        uv_attr.pVertexAttriArray = uv_data;
        uv_attr.VertexAttriArraySize = N * N * sizeof(glm::vec2);
        uv_attr.size = 2;
        uv_attr.stride = 2 * sizeof(float);
        uv_attr.type = GL_FLOAT;
        prim.AddAttribute(uv_attr);

        prim.mode = GL_TRIANGLES;
        prim.count = (N - 1) * (N - 1) * 6;
        prim.pIndexArray = indices_data;
        prim.IndexArraySize = prim.count * sizeof(unsigned int);
        prim.type = GL_UNSIGNED_INT;

        PatchMesh->AddPrimitive(prim);
    }

    SceneObjectHugeSurface::~SceneObjectHugeSurface()
    {
        delete[] pos_data;
        delete[] uv_data;
        delete[] indices_data;
        pos_data = nullptr;
        uv_data = nullptr;
        indices_data = nullptr;
    }

}
