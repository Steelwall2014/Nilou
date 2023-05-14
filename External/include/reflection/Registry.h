#pragma once
#include <map>
#include <string>
#include <memory>
#include "TypeDescriptor.h"

namespace reflection {

    template<typename T>
    struct TClassRegistry { };

    class Registry
    {
    public:
    
        static Registry& Instance()
        {
            static Registry registry;
            return registry;
        }

        void Register(std::unique_ptr<TypeDescriptor> InTypeDescriptor)
        {
            TypeDescriptors[InTypeDescriptor->GetTypeName()] = std::move(InTypeDescriptor);
        }

        static const TypeDescriptor* GetTypeByName(const std::string& Name)
        {
            if (Instance().TypeDescriptors.contains(Name))
                return Instance().TypeDescriptors.at(Name).get();
            return nullptr;
        }
    
    private:

        std::map<std::string, std::unique_ptr<TypeDescriptor>> TypeDescriptors;

    };

    inline bool IsChildOf(const TypeDescriptor *DerivedClass, const TypeDescriptor *BaseClass)
    {
        if (DerivedClass->GetTypeName() == BaseClass->GetTypeName())
            return true;
        std::queue<std::string> q;
        q.push(DerivedClass->GetTypeName());
        while (!q.empty())
        {
            std::string temp_class = q.front(); q.pop();
            auto temp_desc = Registry::GetTypeByName(temp_class);
            for (std::string parent_class : temp_desc->GetParentClasses())
            {
                if (parent_class == BaseClass->GetTypeName())
                    return true;
                q.push(parent_class);
            }
        }
        return false;
    }

}