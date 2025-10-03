#pragma once
#include <iostream>

#include "Common/CoreUObject/Class.h"

namespace nilou {

    class NCLASS A : public NObject
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int a;
    };

    class NCLASS B : public A 
    { 
        GENERATED_BODY()
    public:
        NPROPERTY()
        int b;
    };
}