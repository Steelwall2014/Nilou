#pragma once
#include <iostream>

#include "Common/CoreUObject/Object.h"

namespace nilou {

    class NCLASS A : public UObject
    { 
        GENERATE_BODY()
    public:
        NPROPERTY()
        int a;
    };

    class NCLASS B : public A 
    { 
        GENERATE_BODY()
    public:
        NPROPERTY()
        int b;
    };
}