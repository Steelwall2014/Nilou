#pragma once
#include "RHIResources.h"

namespace nilou {

    class RHICommandContext
    {
    public:

        virtual RHIDescriptorSet* CreateDescriptorSet() = 0;

    };
}