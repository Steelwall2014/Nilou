#pragma once
#include <async++/async++.h>

#include "Platform.h"

namespace nilou {

    enum EThreadPriority
    {
        TPri_Normal,
        TPri_AboveNormal,
        TPri_BelowNormal,
        TPri_Highest,
        TPri_Lowest,
        TPri_SlightlyBelowNormal,
        TPri_TimeCritical,
        TPri_Num,
    };

    class FRunnable
    {
    public:
        virtual bool Init();
        virtual uint32 Run();
        virtual void Stop();
        virtual void Exit();
    };

    class FRunnableThread
    {
        static uint32 RunnableTlsSlot;    // FRunnableThread的TLS插槽索引.

    public:
        static uint32 GetTlsSlot();
        // 静态类, 用于创建线程, 需提供一个FRunnable对象, 用于线程执行的任务.
        static FRunnableThread* Create(FRunnable* InRunnable, const std::string ThreadName, EThreadPriority InThreadPri);

        virtual void SetThreadPriority( EThreadPriority NewPriority );
        // 暂停/继续运行线程
        virtual void Suspend( bool bShouldPause = true );
        // 销毁线程, 通常需要指定等待标记bShouldWait为true, 否则可能引起内存泄漏或死锁!
        virtual bool Kill( bool bShouldWait = true );
        // 等待执行完毕, 会卡调用线程.
        virtual void WaitForCompletion();
        
        const uint32 GetThreadID() const;
        const std::string& GetThreadName() const;

    protected:
        std::string ThreadName;
        FRunnable* Runnable; // 被执行对象
        EThreadPriority ThreadPriority;
        uint32 ThreadID;

    private:
        virtual void Tick();
    };
}