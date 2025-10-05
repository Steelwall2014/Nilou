#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"

namespace nilou {

    class NCLASS NTestObjectA : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int a;
    };

    class NCLASS NTestObjectB : public NTestObjectA 
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int b;
    };
}