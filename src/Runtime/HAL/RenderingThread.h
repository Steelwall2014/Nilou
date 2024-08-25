#pragma once
#include <vector>

#include "Thread.h"
#include "DynamicRHI.h"

#include "Common/Log.h"

namespace nilou {
    class RenderGraph;

    class EnqueueUniqueRenderCommandType
    {
    public:
        using Lambda = std::function<void(RenderGraph&)>;
        EnqueueUniqueRenderCommandType(Lambda &&InLambda, const char *InTraceBackString) 
            : lambda(std::forward<Lambda>(InLambda)) 
            , TraceBackString(InTraceBackString)
        { }

        void DoTask();

    private:
        Lambda lambda;
        const char *TraceBackString;
    };

    class FRenderingThread : public FRunnable
    {
    public:
        virtual bool Init() override;
        virtual uint32 Run() override;
        virtual void Exit() override;

        template <typename STR, typename Lambda>
        void EnqueueRenderCommand(Lambda &&lambda)
        {
            std::lock_guard<std::mutex> lock(mutex);
            RenderCommands.emplace(std::forward<Lambda>(lambda), STR::Str());
        }

        static FRenderingThread *RenderingThread;
        static uint32 GetFrameCount() { return FRenderingThread::FrameCount; }
        static void NotifyEndOfFrame();
        static void NotifyStartOfFrame();
        static RenderGraph& GetRenderGraph() { return *RenderingThread->GraphRecording; }

    private:

        std::mutex mutex;
        std::queue<EnqueueUniqueRenderCommandType> RenderCommands;
        RenderGraph* GraphExucuting;
        RenderGraph* GraphRecording;
        static uint32 FrameCount;

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