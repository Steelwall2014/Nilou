#pragma once
#include <vector>

#include "HAL/Thread.h"
#include "DynamicRHI.h"
#include "RHICommandList.h"
#include "Logging/LogMacros.h"

namespace nilou {
    class RenderGraph;

    class EnqueueUniqueRenderCommandType
    {
    public:
        using Lambda = std::function<void(RenderGraph&)>;
        EnqueueUniqueRenderCommandType() = default;
        EnqueueUniqueRenderCommandType(Lambda &&InLambda, const char *InTraceBackString) 
            : lambda(std::forward<Lambda>(InLambda)) 
            , TraceBackString(InTraceBackString)
        { }

        bool IsValid() const { return lambda != nullptr; }
        RENDERCORE_API void DoTask();

    private:
        Lambda lambda;
        const char *TraceBackString;
    };

    class RENDERCORE_API FRenderingThread final : public FRunnable
    {
    public:
        virtual bool Init() override;
        virtual uint32 Run() override;
        virtual void Stop() override;
        virtual void Exit() override;

        template <typename STR, typename Lambda>
        static void EnqueueRenderCommand(Lambda &&lambda)
        {
            std::lock_guard<std::mutex> lock(mutex);
            RenderCommands.emplace(std::forward<Lambda>(lambda), STR::Str());
        }
        void NotifyEndOfFrame();
        void NotifyStartOfFrame();

        static FRenderingThread *RenderingThread;
        static RenderGraph& GetRenderGraph() { return *RenderingThread->GraphRecording; }

        static std::function<void()> PreRenderThreadInitDelegate;

        ~FRenderingThread();

    private:
        static std::mutex mutex;
        static std::queue<EnqueueUniqueRenderCommandType> RenderCommands;
        std::unique_ptr<RenderGraph> GraphExecuting = nullptr;
        std::unique_ptr<RenderGraph> GraphRecording = nullptr;
        std::atomic<bool> bShouldExit = false;

    };

    RENDERCORE_API bool IsInRenderingThread();

    template <typename STR, typename Lambda>
    void EnqueueUniqueRenderCommand(Lambda &&lambda)
    {
        if (IsInRenderingThread())
        {
            RenderGraph& Graph = FRenderingThread::GetRenderGraph();
            lambda(Graph);
        }
        else
        {
            FRenderingThread::EnqueueRenderCommand<STR>(std::forward<Lambda>(lambda));
        }
    }

    #define ENQUEUE_RENDER_COMMAND(Type) \
        struct Type##Name \
        {  \
            static const char *Str() { return #Type; } \
        }; \
        EnqueueUniqueRenderCommand<Type##Name>
}