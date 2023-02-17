#pragma once
#include "Thread.h"

namespace nilou {

    class FRenderingThread : public FRunnable
    {
    public:
        virtual bool Init();
        virtual uint32 Run();

    };

    bool IsInRenderingThread();
}