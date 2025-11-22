#pragma once
#include "HAL/Platform.h"

namespace nilou {

    /**
     * 崩溃处理器类
     */
    class CORE_API FCrashHandler
    {
    public:
        /**
         * 初始化崩溃处理器
         * 设置未处理异常过滤器和信号处理器
         */
        static void Initialize();

        /**
         * 卸载崩溃处理器
         */
        static void Shutdown();

    private:
        static bool bInitialized;
    };

}

