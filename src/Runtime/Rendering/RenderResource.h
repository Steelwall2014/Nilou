#pragma once

#include <vector>
#include "Thread.h"
#include "RenderingThread.h"
#include "RHIResources.h"
#include "RenderGraphResources.h"

namespace nilou {
    class RenderGraph;
    class RDGBuffer;

    class FRenderResource
    {
    public:

        enum class EInitPhase : uint8
        {
            Pre,
            Default,
            MAX
        };

        // used to create non-persistent resources
        virtual void InitRHI(RenderGraph&) { bRHIInitialized = true; }
        // used to create persistent resources
        virtual void InitRHI() { bRHIInitialized = true; }
        virtual void ReleaseRHI() { bRHIInitialized = false; }
        
        // used to create non-persistent resources
        virtual void InitResource(RenderGraph&);
        // used to create persistent resources
        virtual void InitResource();
        virtual void ReleaseResource();
        virtual ~FRenderResource() { ReleaseResource(); }
        bool IsInitialized() { return bRHIInitialized; }
        void UpdateRHI();
        static std::vector<FRenderResource*>& GetResourceList();

    private:
        int32 ListIndex = -1;
        bool bRHIInitialized = false;
    };

    class FVertexBuffer : public FRenderResource
    {
    public:
        RDGBufferRef VertexBufferRDG;

        uint32 Stride;

        uint32 NumVertices;

        RHIBuffer* GetRHI() const;

    };

    class FIndexBuffer : public FRenderResource
    {
    public:
        RDGBufferRef IndexBufferRDG;

        uint32 Stride;

        uint32 NumIndices;

        RHIBuffer* GetRHI() const;
    };

    #define BeginInitResource(Resource) BeginInitResource_Internal(Resource, __FILE__, __LINE__)

    #define BeginReleaseResource(Resource) BeginReleaseResource_Internal(Resource, __FILE__, __LINE__)

    void BeginInitResource_Internal(FRenderResource* Resource, const char *file, int line);
    void BeginReleaseResource_Internal(FRenderResource* Resource, const char *file, int line);

    std::vector<int32>& GetFreeIndicesList();

    /** Used to declare a render resource that is initialized/released by static initialization/destruction. */
    template<class ResourceType, FRenderResource::EInitPhase InInitPhase = FRenderResource::EInitPhase::Default>
    class TGlobalResource : public ResourceType
    {
    public:
        /** Default constructor. */
        TGlobalResource()
        {
            InitGlobalResource();
        }

        /** Initialization constructor: 1 parameter. */
        template<typename... Args>
        explicit TGlobalResource(Args... InArgs)
            : ResourceType(InArgs...)
        {
            InitGlobalResource();
        }

        /** Destructor. */
        virtual ~TGlobalResource()
        {
            ReleaseGlobalResource();
        }

    private:

        /**
         * Initialize the global resource.
         */
        void InitGlobalResource()
        {
            if (IsInRenderingThread())
            {
                // If the resource is constructed in the rendering thread, directly initialize it.
                ((ResourceType*)this)->InitResource(FRenderingThread::GetRenderGraph());
            }
            else
            {
                // If the resource is constructed outside of the rendering thread, enqueue a command to initialize it.
                BeginInitResource((ResourceType*)this);
            }
        }

        /**
         * Release the global resource.
         */
        void ReleaseGlobalResource()
        {
            // This should be called in the rendering thread, or at shutdown when the rendering thread has exited.
            // However, it may also be called at shutdown after an error, when the rendering thread is still running.
            // To avoid a second error in that case we don't assert.
    #if 0
            check(IsInRenderingThread());
    #endif

            // Cleanup the resource.
            ((ResourceType*)this)->ReleaseResource();
        }
    };

}