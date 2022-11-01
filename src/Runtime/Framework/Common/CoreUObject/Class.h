#pragma once

#include <string>
#include "MarkedClasses.generated.h"

namespace nilou {
    
    class UClass
    {
    public:
        UClass() :ClassEnum(EUClasses::MC_None), ClassName("") {}
        UClass(const std::string &InName, EUClasses InClassEnum) :ClassEnum(InClassEnum), ClassName(InName) {}

        bool IsChildOf(const UClass *BaseClass) const;

        inline bool operator==(const UClass &Other) const
        {
            return ClassEnum == Other.ClassEnum;
        }

        inline bool operator<(const UClass &Other) const
        {
            return ClassEnum < Other.ClassEnum;
        }

        std::string ClassName;
        EUClasses ClassEnum;
    };

}