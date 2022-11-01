#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
    const char *IndexDataType_String[] = {
        "INT16",
        "INT32"
    };
    enum IndexDataType {
        kIndexDataTypeInt16,
        kIndexDataTypeInt32,
    };
    std::ostream &operator<<(std::ostream &out, IndexDataType type)
    {
        out << IndexDataType_String[type];
        return out;
    }

    // ห๗าýสýื้
    class SceneObjectIndexArray : public BaseSceneObject
    {
    protected:
        uint32_t    m_MaterialIndex;
        IndexDataType m_DataType;

        union {
            uint16_t *m_pDataI16;
            uint32_t *m_pDataI32;
        };
    };
}