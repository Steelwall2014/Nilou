#pragma once
#include "TypeDescriptor.h"
#include "Registry.h"

namespace reflection {

    class NClass
    {
    public:

        template<typename T>
        friend class TClassRegistry;

        NClass() = default;

        bool IsChildOf(const NClass *BaseClass) const
        {
            if (Type->GetTypeName() == BaseClass->Type->GetTypeName())
                return true;
            std::queue<std::string> q;
            q.push(Type->GetTypeName());
            while (!q.empty())
            {
                std::string temp_class = q.front(); q.pop();
                auto temp_desc = Registry::GetTypeByName(temp_class);
                for (std::string parent_class : temp_desc->GetParentClasses())
                {
                    if (parent_class == BaseClass->Type->GetTypeName())
                        return true;
                    q.push(parent_class);
                }
            }
            return false;
        }

        inline bool operator==(const NClass &Other) const
        {
            return Type == Other.Type;
        }

        inline bool operator<(const NClass &Other) const
        {
            return Type < Other.Type;
        }

        const reflection::TypeDescriptor *GetTypeDescriptor() const
        {
            return Type;
        }

    private:

        const reflection::TypeDescriptor *Type;

    };

}