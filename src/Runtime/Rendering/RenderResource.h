#pragma once

#include "RHIResources.h"
#include "glm/glm.hpp"
#include <vector>

namespace nilou {
    class RenderGraph;
    class RDGBuffer;
    using RDGBufferRef = std::shared_ptr<RDGBuffer>;

    class FRenderResource
    {
    public:
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
}