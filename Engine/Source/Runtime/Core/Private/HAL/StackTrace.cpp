#include "HAL/StackTrace.h"
#include "Logging/LogMacros.h"

#include <windows.h>
#include <dbghelp.h>
#include <sstream>

#pragma comment(lib, "dbghelp.lib")

namespace nilou {

    // 单例模式，确保符号处理器只初始化一次
    struct FSymbolHelper
    {
        static FSymbolHelper& Get()
        {
            static FSymbolHelper Instance;
            return Instance;
        }

        FSymbolHelper()
            : bInitialized(false)
        {
            Initialize();
        }

        ~FSymbolHelper()
        {
            if (bInitialized)
            {
                SymCleanup(GetCurrentProcess());
            }
        }

        bool Initialize()
        {
            if (bInitialized)
            {
                return true;
            }

            HANDLE Process = GetCurrentProcess();
            
            // 设置选项：加载行号信息，取消路径修饰
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);

            // 初始化符号处理器
            if (SymInitialize(Process, nullptr, TRUE))
            {
                bInitialized = true;
                return true;
            }

            return false;
        }

        bool bInitialized;
    };

    std::vector<FStackFrame> FStackTrace::CaptureStack(int32 MaxDepth)
    {
        std::vector<FStackFrame> StackFrames;

        // 初始化符号处理器
        FSymbolHelper::Get().Initialize();

        // 捕获堆栈回溯
        void* BackTrace[62];
        USHORT Frames = CaptureStackBackTrace(0, MaxDepth, BackTrace, nullptr);

        if (Frames == 0)
        {
            return StackFrames;
        }

        HANDLE Process = GetCurrentProcess();

        // 为符号信息分配空间（需要足够大以容纳SYMBOL_INFO结构）
        const int32 SymbolInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR);
        char* SymbolBuffer = new char[SymbolInfoSize];
        SYMBOL_INFO* SymbolInfo = reinterpret_cast<SYMBOL_INFO*>(SymbolBuffer);

        // 为行号信息分配空间
        IMAGEHLP_LINE64 LineInfo = { 0 };
        LineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        DWORD Displacement = 0;

        for (USHORT i = 0; i < Frames; ++i)
        {
            FStackFrame Frame;
            Frame.Address = reinterpret_cast<uint64>(BackTrace[i]);

            // 初始化SYMBOL_INFO结构
            SymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
            SymbolInfo->MaxNameLen = MAX_SYM_NAME;

            // 从地址获取符号信息
            if (SymFromAddr(Process, Frame.Address, reinterpret_cast<PDWORD64>(&Frame.Offset), SymbolInfo))
            {
                Frame.FunctionName = SymbolInfo->Name;
            }
            else
            {
                Frame.FunctionName = "<Unknown>";
            }

            // 从地址获取行号信息
            if (SymGetLineFromAddr64(Process, Frame.Address, &Displacement, &LineInfo))
            {
                Frame.FileName = LineInfo.FileName;
                Frame.LineNumber = static_cast<int32>(LineInfo.LineNumber);
            }
            else
            {
                Frame.FileName = "<Unknown>";
                Frame.LineNumber = 0;
            }

            // 获取模块名称
            HMODULE ModuleHandle = nullptr;
            if (GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCTSTR>(BackTrace[i]),
                &ModuleHandle))
            {
                char ModuleName[MAX_PATH];
                if (GetModuleFileNameA(ModuleHandle, ModuleName, MAX_PATH))
                {
                    // 只保留文件名，去掉路径
                    const char* FileName = strrchr(ModuleName, '\\');
                    if (FileName)
                    {
                        Frame.ModuleName = FileName + 1;
                    }
                    else
                    {
                        Frame.ModuleName = ModuleName;
                    }
                }
                else
                {
                    Frame.ModuleName = "<Unknown>";
                }
            }
            else
            {
                Frame.ModuleName = "<Unknown>";
            }

            StackFrames.push_back(Frame);
        }

        delete[] SymbolBuffer;
        return StackFrames;
    }

    std::string FStackTrace::ToString(const std::vector<FStackFrame>& StackFrames)
    {
        std::stringstream Stream;
        Stream << "\n========== Stack Trace ==========\n";

        for (size_t i = 0; i < StackFrames.size(); ++i)
        {
            const FStackFrame& Frame = StackFrames[i];
            Stream << "[" << i << "] ";

            if (!Frame.ModuleName.empty())
            {
                Stream << Frame.ModuleName << "!";
            }

            if (!Frame.FunctionName.empty())
            {
                Stream << Frame.FunctionName;
            }
            else
            {
                Stream << "0x" << std::hex << Frame.Address << std::dec;
            }

            if (Frame.Offset > 0)
            {
                Stream << " +0x" << std::hex << Frame.Offset << std::dec;
            }

            if (!Frame.FileName.empty() && Frame.LineNumber > 0)
            {
                Stream << "\n    " << Frame.FileName << ":" << Frame.LineNumber;
            }

            Stream << "\n";
        }

        Stream << "==============================\n";
        return Stream.str();
    }

    void FStackTrace::LogStackTrace(const std::vector<FStackFrame>& StackFrames)
    {
        std::string StackTraceStr = ToString(StackFrames);
        NILOU_LOG(Error, "{}", StackTraceStr);
    }

}

