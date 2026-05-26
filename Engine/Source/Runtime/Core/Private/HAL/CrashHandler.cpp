#include "HAL/CrashHandler.h"
#include "HAL/StackTrace.h"
#include "Logging/LogMacros.h"

#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace nilou {

    bool FCrashHandler::bInitialized = false;

    const char* GetExceptionTypeString(DWORD ExceptionCode)
    {
        switch (ExceptionCode)
        {
        case EXCEPTION_ACCESS_VIOLATION:
            return "Access Violation";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            return "Array Bounds Exceeded";
        case EXCEPTION_BREAKPOINT:
            return "Breakpoint";
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            return "Datatype Misalignment";
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            return "Floating Point Denormal Operand";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            return "Floating Point Divide By Zero";
        case EXCEPTION_FLT_INEXACT_RESULT:
            return "Floating Point Inexact Result";
        case EXCEPTION_FLT_INVALID_OPERATION:
            return "Floating Point Invalid Operation";
        case EXCEPTION_FLT_OVERFLOW:
            return "Floating Point Overflow";
        case EXCEPTION_FLT_STACK_CHECK:
            return "Floating Point Stack Check";
        case EXCEPTION_FLT_UNDERFLOW:
            return "Floating Point Underflow";
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "Illegal Instruction";
        case EXCEPTION_IN_PAGE_ERROR:
            return "In Page Error";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            return "Integer Divide By Zero";
        case EXCEPTION_INT_OVERFLOW:
            return "Integer Overflow";
        case EXCEPTION_INVALID_DISPOSITION:
            return "Invalid Disposition";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            return "Noncontinuable Exception";
        case EXCEPTION_PRIV_INSTRUCTION:
            return "Privileged Instruction";
        case EXCEPTION_SINGLE_STEP:
            return "Single Step";
        case EXCEPTION_STACK_OVERFLOW:
            return "Stack Overflow";
        default:
            return "Unknown Exception";
        }
    }

    bool WriteFullDump(EXCEPTION_POINTERS* ExceptionInfo, const char* DumpFilePath)
    {
        HANDLE hFile = CreateFileA(
            DumpFilePath,
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        MINIDUMP_EXCEPTION_INFORMATION ExceptionParam;
        ExceptionParam.ThreadId = GetCurrentThreadId();
        ExceptionParam.ExceptionPointers = ExceptionInfo;
        ExceptionParam.ClientPointers = FALSE;

        // Generate full dump with all memory
        MINIDUMP_TYPE DumpType = static_cast<MINIDUMP_TYPE>(
            MiniDumpWithFullMemory |              // Include all memory
            MiniDumpWithDataSegs |                // Include data segments
            MiniDumpWithHandleData |              // Include handle data
            MiniDumpWithUnloadedModules |        // Include unloaded modules
            MiniDumpWithIndirectlyReferencedMemory | // Include indirectly referenced memory
            MiniDumpWithProcessThreadData |       // Include process and thread data
            MiniDumpWithThreadInfo);              // Include thread information

        bool Success = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            DumpType,
            ExceptionInfo ? &ExceptionParam : nullptr,
            nullptr,
            nullptr);

        CloseHandle(hFile);
        return Success;
    }

    std::string GenerateDumpFileName()
    {
        std::time_t CurrentTime = std::time(nullptr);
        std::tm TimeInfo;
        localtime_s(&TimeInfo, &CurrentTime);

        std::ostringstream FileName;
        FileName << "crash_dump_"
                 << std::put_time(&TimeInfo, "%Y%m%d_%H%M%S")
                 << ".dmp";
        return FileName.str();
    }

    void HandleCrash(EXCEPTION_POINTERS* ExceptionInfo, const char* ExceptionType)
    {
        if (ExceptionInfo == nullptr)
        {
            return;
        }

        const EXCEPTION_RECORD* ExceptionRecord = ExceptionInfo->ExceptionRecord;
        const CONTEXT* Context = ExceptionInfo->ContextRecord;

        // 打印异常信息
        NILOU_LOG(Error, "========================================");
        NILOU_LOG(Error, "Application Crash!");
        NILOU_LOG(Error, "Exception Type: {}", ExceptionType);
        NILOU_LOG(Error, "Exception Code: 0x{:08X}", ExceptionRecord->ExceptionCode);

        // 对于访问违规，打印更多信息
        if (ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
        {
            void* FaultAddress = reinterpret_cast<void*>(ExceptionRecord->ExceptionInformation[1]);
            bool IsWrite = ExceptionRecord->ExceptionInformation[0] != 0;
            NILOU_LOG(Error, "Access Violation Type: {}", IsWrite ? "Write" : "Read");
            NILOU_LOG(Error, "Fault Address: 0x{:016X}", reinterpret_cast<uint64>(FaultAddress));
        }

        // 打印指令指针
#if defined(_WIN64)
        NILOU_LOG(Error, "Instruction Pointer (RIP): 0x{:016X}", Context->Rip);
        NILOU_LOG(Error, "Stack Pointer (RSP): 0x{:016X}", Context->Rsp);
        NILOU_LOG(Error, "Base Pointer (RBP): 0x{:016X}", Context->Rbp);
#else
        NILOU_LOG(Error, "Instruction Pointer (EIP): 0x{:08X}", Context->Eip);
        NILOU_LOG(Error, "Stack Pointer (ESP): 0x{:08X}", Context->Esp);
        NILOU_LOG(Error, "Base Pointer (EBP): 0x{:08X}", Context->Ebp);
#endif

        // 捕获并打印堆栈跟踪
        NILOU_LOG(Error, "Capturing stack trace...");
        
        try
        {
            std::vector<FStackFrame> StackFrames = FStackTrace::CaptureStack(62);
            FStackTrace::LogStackTrace(StackFrames);
        }
        catch (...)
        {
            NILOU_LOG(Error, "Exception occurred while capturing stack trace!");
        }

        NILOU_LOG(Error, "========================================");

        // Generate full dump file
        NILOU_LOG(Error, "Generating full dump file...");
        try
        {
            std::string DumpFileName = GenerateDumpFileName();
            if (WriteFullDump(ExceptionInfo, DumpFileName.c_str()))
            {
                NILOU_LOG(Error, "Full dump saved to: {}", DumpFileName);
            }
            else
            {
                NILOU_LOG(Error, "Failed to generate full dump file");
            }
        }
        catch (...)
        {
            NILOU_LOG(Error, "Exception occurred while generating dump file!");
        }

        // 尝试将堆栈跟踪写入文件
        try
        {
            FILE* File = nullptr;
            if (fopen_s(&File, "crash_report.txt", "w") == 0 && File != nullptr)
            {
                fprintf(File, "Application Crash Report\n");
                fprintf(File, "====================\n\n");
                fprintf(File, "Exception Type: %s\n", ExceptionType);
                fprintf(File, "Exception Code: 0x%08lX\n\n", static_cast<unsigned long>(ExceptionRecord->ExceptionCode));
                
                std::vector<FStackFrame> StackFrames = FStackTrace::CaptureStack(62);
                std::string StackTraceStr = FStackTrace::ToString(StackFrames);
                fprintf(File, "%s", StackTraceStr.c_str());
                
                fclose(File);
                NILOU_LOG(Error, "Crash report saved to crash_report.txt");
            }
        }
        catch (...)
        {
            // 忽略文件写入错误
        }
    }

    // Windows-specific structured exception handler
    static LONG WINAPI StructuredExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
    {
        if (ExceptionInfo == nullptr || ExceptionInfo->ExceptionRecord == nullptr)
        {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        DWORD ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
        const char* ExceptionType = GetExceptionTypeString(ExceptionCode);
        HandleCrash(ExceptionInfo, ExceptionType);

        // Return EXCEPTION_EXECUTE_HANDLER to terminate the program
        // Return EXCEPTION_CONTINUE_SEARCH to continue searching for other handlers
        // Here we terminate the program after printing information
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void FCrashHandler::Initialize()
    {
        if (bInitialized)
        {
            return;
        }

        // Set unhandled exception filter (Windows-specific)
        SetUnhandledExceptionFilter(StructuredExceptionHandler);

        // 设置错误模式，避免显示错误对话框
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

        // 设置控制台处理程序（用于控制台应用）
        SetConsoleCtrlHandler(nullptr, TRUE);

        bInitialized = true;
        NILOU_LOG(Display, "Crash handler initialized");
    }

    void FCrashHandler::Shutdown()
    {
        if (!bInitialized)
        {
            return;
        }

        SetUnhandledExceptionFilter(nullptr);
        bInitialized = false;
    }

}

