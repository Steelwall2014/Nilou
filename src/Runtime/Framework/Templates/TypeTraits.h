#pragma once

namespace nilou {

    template <class DerivedType, class BaseType>
    struct TIsDerivedFrom
    {
        typedef char No[1];
        typedef char Yes[2];

        static Yes &Test(BaseType *);
        static Yes &Test(const BaseType *);
        static No &Test(...);

        static DerivedType *DerivedTypePtr() { return nullptr; }

        static constexpr bool Value = sizeof(Test(DerivedTypePtr())) == sizeof(Yes);

        static constexpr bool IsDerived = Value;
    };

    template<typename T> struct MemberPointerTraits;
    template<typename T, typename U>
    struct MemberPointerTraits<T U::*> {
        using Object = U;
        using Value = T;
    };

}