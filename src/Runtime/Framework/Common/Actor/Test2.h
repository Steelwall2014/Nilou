#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"

namespace nilou {
    class NTestObjectA;

    class NCLASS NTestObjectC : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int c;

        NPROPERTY()
        NTestObjectA* Recursive;
    };
}