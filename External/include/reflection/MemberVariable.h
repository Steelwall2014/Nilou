#pragma once
#include <functional>
#include <any>
#include <string>

namespace reflection {
    class MemberVariable
    {
    public:

        friend class RawTypeDescriptorBuilder;

        template<typename TClass, typename TVar>
        MemberVariable(TVar TClass::* Var)
        {
            getter = [Var](std::any Object) -> std::any {
                return std::any_cast<const TClass*>(Object)->*Var;
            };

            setter = [Var](std::any Object, std::any Value) {
                TClass* self = std::any_cast<TClass*>(Object);
                self->*Var = std::any_cast<TVar>(Value);
            };
        }

        const std::string& GetMemberName() const 
        { 
            return MemberName; 
        }

        template<typename TClass, typename TVar>
        TVar GetValue(const TClass& Object)
        {
            return std::any_cast<TVar>(getter(Object));
        }

        template<typename TClass, typename TVar>
        void SetValue(TClass& Object, TVar Value)
        {
            setter(&Object, Value);
        }

    private:

        std::string MemberName;

        std::function<std::any(std::any)> getter;

        std::function<void(std::any,std::any)> setter;

    };
}