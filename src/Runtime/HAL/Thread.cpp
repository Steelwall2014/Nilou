#include <windows.h>
#include <processthreadsapi.h>

#include "Thread.h"


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
    }

    void FRunnableThread::Kill()
    {
        HANDLE handle = Thread.native_handle();
        TerminateThread(handle, 0);
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