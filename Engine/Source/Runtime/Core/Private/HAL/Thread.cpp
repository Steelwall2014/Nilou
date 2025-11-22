#include <windows.h>
#include <processthreadsapi.h>
#include <windows.h>

#include "HAL/Thread.h"
#include "Logging/LogMacros.h"

void SetThreadName(HANDLE hThread, const wchar_t* name) 
{
    ::SetThreadDescription(hThread, name);
}

namespace nilou {

    std::thread::id GGameThreadId = std::this_thread::get_id();

    std::unique_ptr<FRunnableThread> FRunnableThread::Create(FRunnable *InRunnable, const std::string &InThreadName/*, EThreadPriority InThreadPri*/)
    {
        std::unique_ptr<FRunnableThread> NewThread = std::make_unique<FRunnableThread>();
        NewThread->CreateInternal(InRunnable, InThreadName);
        return NewThread;
    }

    void FRunnableThread::CreateInternal(FRunnable *InRunnable, const std::string &InThreadName)
    {
        this->Runnable = InRunnable;
        this->ThreadName = InThreadName;
        auto f = [this]() {
            std::thread::id id = std::this_thread::get_id();
            this->ThreadID = id;
            this->Runnable->Init();
            this->bRunnableInitialized = true;
            this->Runnable->Run();
            this->Runnable->Exit();
            this->bRunnableExited = true;
        };
        Thread = std::thread(f);
        SetThreadName(Thread.native_handle(), encode::utf8ToWide(InThreadName).c_str());
    }

    void FRunnableThread::Kill()
    {
        if (Runnable)
        {
            Runnable->Stop();
        }
        Thread.join();
    }

    void FRunnableThread::WaitForCompletion()
    {
        Thread.join();
    }

    bool IsInGameThread()
    {
        return std::this_thread::get_id() == GGameThreadId;
    }

}