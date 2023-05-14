#include "Object.h"

namespace nilou {

    bool UObject::IsA(const NClass *Class)
    {
        return GetClass()->IsChildOf(Class);
    }

    const std::string& UObject::GetClassName() const 
    {
        return GetClass()->GetTypeDescriptor()->GetTypeName();
    }

    void* CreateDefaultObjectByName(const std::string &ClassName)
    {
        auto Type = reflection::Registry::GetTypeByName(ClassName);
        void* object = Type->GetDefaultConstructor().Invoke();
        return object;
    }
}