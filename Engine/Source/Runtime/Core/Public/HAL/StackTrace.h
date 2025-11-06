#pragma once

#include <vector>
#include <string>
#include "HAL/Platform.h"

namespace nilou {

    /**
     * 堆栈跟踪信息结构
     */
    struct FStackFrame
    {
        /** 模块名称 */
        std::string ModuleName;
        /** 函数名称 */
        std::string FunctionName;
        /** 文件名 */
        std::string FileName;
        /** 行号 */
        int32 LineNumber;
        /** 地址 */
        uint64 Address;
        /** 相对于模块的偏移 */
        uint64 Offset;

        FStackFrame()
            : LineNumber(0)
            , Address(0)
            , Offset(0)
        {
        }
    };

    /**
     * 堆栈跟踪工具类
     */
    class FStackTrace
    {
    public:
        /**
         * 捕获当前线程的堆栈跟踪
         * @param MaxDepth 最大堆栈深度
         * @return 堆栈帧列表
         */
        static std::vector<FStackFrame> CaptureStack(int32 MaxDepth = 62);

        /**
         * 将堆栈跟踪格式化为字符串
         * @param StackFrames 堆栈帧列表
         * @return 格式化的堆栈字符串
         */
        static std::string ToString(const std::vector<FStackFrame>& StackFrames);

        /**
         * 将堆栈跟踪直接输出到日志
         * @param StackFrames 堆栈帧列表
         */
        static void LogStackTrace(const std::vector<FStackFrame>& StackFrames);
    };

}

