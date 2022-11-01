#pragma once
#include "Common/BaseSceneObject.h"
#include "Common/SceneObject/SceneObjectMaterial.h"
//#include "SceneObjectVertexArray.h"
//#include "SceneObjectIndexArray.h"
#include <tinygltf/tiny_gltf.h>

namespace und {
    // glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    // glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *offset)

    class SceneObjectMesh : public BaseSceneObject
    {
    public:
        struct AttriPointer
        {
            std::string name;
            int size;                   
            /***************
            type         size
            "SCALAR"        1
            "VEC2"          2
            "VEC3"          3
            "VEC4"          4
            "MAT2"          4
            "MAT3"          9
            "MAT4"         16
            *****************/

            int type;                   
            /********
            5120 BYTE
            5121 UNSIGNED_BYTE
            5122 SHORT
            5123 UNSIGNED_SHORT
            5125 UNSIGNED_INT
            5126 FLOAT
            ********/

            bool normalized;            // 是否进行标准化
            int stride;
            size_t offset;
            void *pVertexAttriArray;
            int  VertexAttriArraySize;       // vertex array的字节数
            friend std::ostream &operator<<(std::ostream &out, const AttriPointer &obj);

        };
        
        // 对应gltf中mesh primitives，每一个primitive都定义了一个需要进行渲染的geometry
        struct Primitive
        {
            void *pIndexArray;
            unsigned int IndexArraySize;     // index array的字节数
            unsigned int mode;               // glDrawElement第一个参数，GL_TRIANGLES等
            unsigned int count;              // glDrawElement第二个参数，意思是索引数组有多少个元素
            unsigned int type;               // glDrawElement第三个参数，意思是索引数组的数据类型
            std::vector<AttriPointer> Attributes;
            std::shared_ptr<SceneObjectMaterial> Material;
            void AddAttribute(AttriPointer &attr);
            friend std::ostream &operator<<(std::ostream &out, const Primitive &obj);
        };
    protected:    

        //std::vector<SceneObjectIndexArray>  m_IndexArray;
        //std::vector<SceneObjectVertexArray> m_VertexArray;

        //bool        m_bVisible = true;
        //bool        m_bShadow = false;
        //bool        m_bMotionBlur = false;
        std::vector<SceneObjectMesh::Primitive> m_Primitives;

    public:

        SceneObjectMesh() : BaseSceneObject(SceneObjectType::kSceneObjectTypeMesh) {};
        SceneObjectMesh(tinygltf::Mesh &mesh);
        void AddPrimitive(const SceneObjectMesh::Primitive &primitive);
        const std::vector<SceneObjectMesh::Primitive> &GetPrimitives() const;

        virtual void dump(std::ostream &out);
        //friend std::ostream &operator<<(std::ostream &out, const SceneObjectMesh &obj);
    };

}