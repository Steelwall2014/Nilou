#pragma once
#include <string>

#include <thread_pool/BS_thread_pool.hpp>

#include "Platform.h"

namespace nilou {

    // enum EThreadPriority
    // {
    //     TPri_Normal,
    //     TPri_AboveNormal,
    //     TPri_BelowNormal,
    //     TPri_Highest,
    //     TPri_Lowest,
    //     TPri_SlightlyBelowNormal,
    //     TPri_TimeCritical,
    //     TPri_Num,
    // };

    class FRunnable
    {
    public:
        /**
         * Initializes the runnable object.
         *
         * This method is called in the context of the thread object that aggregates this, not the
         * thread that passes this runnable to a new thread.
         *
         * @return True if initialization was successful, false otherwise
         * @see Run, Stop, Exit
         */
        virtual bool Init()
        {
            return true;
        }

        /**
         * Runs the runnable object.
         *
         * This is where all per object thread work is done. This is only called if the initialization was successful.
         *
         * @return The exit code of the runnable object
         * @see Init, Stop, Exit
         */
        virtual uint32 Run() = 0;

        /**
         * Stops the runnable object.
         *
         * This is called if a thread is requested to terminate early.
         * @see Init, Run, Exit
         */
        virtual void Stop() { }

        /**
         * Exits the runnable object.
         *
         * Called in the context of the aggregating thread to perform any cleanup.
         * @see Init, Run, Stop
         */
        virtual void Exit() { }
    };

    class FRunnableThread
    {
    public:
        static std::unique_ptr<FRunnableThread> Create(FRunnable* InRunnable, const std::string &InThreadName/*, EThreadPriority InThreadPri*/);

        // virtual void SetThreadPriority( EThreadPriority NewPriority );
        // virtual void Suspend( bool bShouldPause = true );
        void Kill();
        void WaitForCompletion();
        
        const std::thread::id GetThreadID() const { return ThreadID; }
        const std::string& GetThreadName() const { return ThreadName; }

        bool IsRunnableInitialized() const { return bRunnableInitialized; }

        bool IsRunnableExited() const { return bRunnableExited; }

        ~FRunnableThread() { delete Runnable; }

    protected:
        std::string ThreadName;
        FRunnable* Runnable;
        // EThreadPriority ThreadPriority;
        std::thread::id ThreadID;
        std::thread Thread;

        std::atomic<bool> bRunnableInitialized = false;

        std::atomic<bool> bRunnableExited = false;

    private:
        void CreateInternal(FRunnable *InRunnable, const std::string &InThreadName);
        // virtual void Tick();
    };

    bool IsInGameThread();

    bool IsInRenderingThread();
}