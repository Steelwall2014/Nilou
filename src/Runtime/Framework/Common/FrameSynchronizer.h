#pragma once
#include <mutex>
#include "Platform.h"

namespace nilou {

    class FFrameSynchronizer
    {
    public:
        static std::condition_variable cv;
        static std::mutex mutex;
        static std::condition_variable render_cv;
        static std::mutex render_mutex;
        static std::condition_variable main_cv;
        static std::mutex main_mutex;
        static bool ShouldRenderingThreadLoopRun;
        static bool ShouldRenderingThreadWait;
        static bool ShouldMainThreadWait;
        static uint32 MainThreadFrameCount;
        static uint32 RenderingThreadFrameCount;
    };

}