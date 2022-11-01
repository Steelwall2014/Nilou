#pragma once
#include "Common/BaseSceneObject.h"

namespace und {

    const char *VertexDataType_String[] = {
        "FLOAT",
        "DOUBLE"
        };
    enum VertexDataType {
        kVertexDataTypeFloat,
        kVertexDataTypeDouble
    };
    std::ostream &operator<<(std::ostream &out, VertexDataType type)
    {
        out << VertexDataType_String[type];
        return out;
    }

    // ¶¥µãÊý×é
    class SceneObjectVertexArray : public BaseSceneObject
    {
    protected:
        std::string m_Attribute;
        VertexDataType m_DataType;

        union {
            float *m_pDataFloat;
            double *m_pDataDouble;
        };
        size_t      m_szData;
    };
}
