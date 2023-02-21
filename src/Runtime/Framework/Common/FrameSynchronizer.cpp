#include "FrameSynchronizer.h"

namespace nilou {
    std::condition_variable FFrameSynchronizer::cv;
    std::mutex FFrameSynchronizer::mutex;
    std::condition_variable FFrameSynchronizer::main_cv;
    std::mutex FFrameSynchronizer::main_mutex;
    std::condition_variable FFrameSynchronizer::render_cv;
    std::mutex FFrameSynchronizer::render_mutex;
    bool FFrameSynchronizer::ShouldRenderingThreadLoopRun = false;
    bool FFrameSynchronizer::ShouldRenderingThreadWait = false;
    bool FFrameSynchronizer::ShouldMainThreadWait = false;
    uint32 FFrameSynchronizer::MainThreadFrameCount = 0;
    uint32 FFrameSynchronizer::RenderingThreadFrameCount = 0;
}