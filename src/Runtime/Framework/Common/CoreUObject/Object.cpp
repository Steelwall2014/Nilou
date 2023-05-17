#include "Object.h"

namespace nilou {

    bool UObject::IsA(const NClass *Class)
    {
        return GetClass()->IsChildOf(Class);
    }

    std::string_view UObject::GetClassName() const 
    {
        return GetClass()->GetType().GetName();
    }

    UObject* CreateDefaultObjectByName(const std::string &ClassName)
    {
        auto Object = Ubpa::UDRefl::Mngr.New(Ubpa::Type(ClassName));
        auto BaseObject = Object.StaticCast(Ubpa::Type_of<UObject>);
        UObject* pObject = BaseObject.AsPtr<UObject>();
        if (pObject) 
            return pObject;
        Ubpa::UDRefl::Mngr.Delete(Object);
        return nullptr;
    }
}