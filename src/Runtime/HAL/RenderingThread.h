#pragma once
#include <vector>

#include "Thread.h"
#include "DynamicRHI.h"

#include "Common/Log.h"

namespace nilou {

    class EnqueueUniqueRenderCommandType
    {
    public:
        using Lambda = std::function<void(FDynamicRHI*)>;
        EnqueueUniqueRenderCommandType(Lambda &&InLambda, const char *InTraceBackString) 
            : lambda(std::forward<Lambda>(InLambda)) 
            , TraceBackString(InTraceBackString)
        { }

        void DoTask()
        {
            NILOU_LOG(Info, "Enqueued render command: " + std::string(TraceBackString))
            lambda(FDynamicRHI::GetDynamicRHI());
        }

    private:
        Lambda lambda;
        const char *TraceBackString;
    };

    class FRenderingThread : public FRunnable
    {
    public:
        virtual bool Init();
        virtual uint32 Run();

        template <typename STR, typename Lambda>
        void EnqueueRenderCommand(Lambda &&lambda)
        {
            mutex.lock();
            RenderCommands.emplace(lambda, STR::Str());
            mutex.unlock();
        }

        static FRenderingThread *RenderingThread;

    private:

        std::mutex mutex;
        std::queue<EnqueueUniqueRenderCommandType> RenderCommands;

    };

    template <typename STR, typename Lambda>
    void EnqueueUniqueRenderCommand(Lambda &&lambda)
    {
        if (IsInRenderingThread())
        {
            lambda(FDynamicRHI::GetDynamicRHI());
        }
        else
        {
            FRenderingThread::RenderingThread->EnqueueRenderCommand<STR>(std::forward<Lambda>(lambda));
        }
    }

    #define ENQUEUE_RENDER_COMMAND(Type) \
        struct Type##Name \
        {  \
            static const char *Str() { return #Type; } \
        }; \
        EnqueueUniqueRenderCommand<Type##Name>
}